#!/bin/env python3


import sys
from pathlib import Path


##############################################################################

from lark import Lark, Transformer, v_args

json_grammar = r"""
    ?start: value

    ?value: object
          | array
          | string
          | SIGNED_NUMBER      -> number
          | "true"             -> true
          | "false"            -> false
          | "null"             -> null

    array  : ["json::array_t"] "{" value ("," value)* "}"
    object : "{" pair ("," pair)* "}"
    pair   : "{" string "," value "}"

    string : ESCAPED_STRING

    %import common.ESCAPED_STRING
    %import common.SIGNED_NUMBER
    %import common.WS

    %ignore WS
"""


class TreeToJson(Transformer):
    @v_args(inline=True)
    def string(self, s):
        return s[1:-1].replace('\\"', '"')

    array = list
    pair = tuple
    object = dict

    @v_args(inline=True)
    def number(self, s):
        try:
            return int(s)
        except ValueError:
            try:
                return float(s)
            except ValueError:
                raise ValueError(f"Not a number: {s}")

    null = lambda self, _: None
    true = lambda self, _: True
    false = lambda self, _: False


### Create the "JSON" parser with Lark, using the LALR algorithm
spec_parser = Lark(json_grammar, parser='lalr',
                   lexer='basic',
                   propagate_positions=False,
                   maybe_placeholders=False,
                   transformer=TreeToJson())

##############################################################################

def parse_file(path, sections, schemas):
    print("Parsing:", path)

    schema_started = False
    section_started = False
    strings_started = False
    section_id = ""
    section_title = ""
    schema_id = ""
    ext_schema_id = ""
    schema = ""
    section = ""
    strings = ""

    with open(path) as f:
        for line in f:
            l = line.strip()
            if l.startswith("//_DOCS: SECTION_START"):
                section_started = True
                pos1 = len("//_DOCS: SECTION_START")
                pos2 = l.find(" ", pos1 + 1)
                section_id = l[pos1 : pos2].strip()
                section_title = l[pos2:].strip()
            elif l.startswith("//_DOCS: SCHEMA_START"):
                schema_started = True
                schema_id = l[l.rfind(" ") + 1 :]
                ext_schema_id = None
            elif l.startswith("//_DOCS: SCHEMA_INCLUDE"):
                ext_schema_id = l[l.rfind(" ") + 1 :]
            elif l.startswith("//_DOCS: STRINGS_START"):
                strings_started = True
            elif l.startswith("//_DOCS: END"):
                strings_started = False
                section_started = False
                schema_started = False
            elif l.startswith("/*") or l.startswith("*/"):
                continue
            else:
                if schema_started:
                    schema += line
                elif section_started:
                    section += line
                elif strings_started:
                    strings += line

    ostrings = {}
    str_id = None
    for line in strings.splitlines():
        if "_DOCS_" in line:
            str_id = line[line.find("_DOCS_") : line.rfind(" ")]
            ostrings[str_id] = ""
        elif str_id is not None:
            if line.endswith(";"):
                ostrings[str_id] += line.strip()[1 : -2]
                str_id = None
            else:
                ostrings[str_id] += line.strip()[1 : -1]

    if schema_id:
        tmp = ""
        for line in schema.splitlines():
            if "_DOCS_" in line:
                str_id = line[line.find("_DOCS_") : line.rfind("}")]
                tmp += line.replace(str_id, '"' + ostrings[str_id] + '"') + "\n"
            else:
                tmp += line + "\n"
        schema = tmp[tmp.find("{") : tmp.rfind("}") + 1]

        print(schema)
        if schema:
            oschema = spec_parser.parse(schema)
        else:
            oschema = {}

        schemas[schema_id] = {
            "id": schema_id,
            "schema": oschema,
            "ext_schema_id": ext_schema_id
        }

    if section_id:
        sections[section_id] = {
            "id": section_id,
            "title": section_title,
            "body": section,
        }


def generate_rst(path, section, schema, common_schema):
    with open(path, "a") as f:
        f.write(".. _{}:\n".format(section["title"]))
        f.write(".. _{}s:\n\n".format(section["title"]))

        f.write("+" * len(section["title"]))
        f.write("\n")
        f.write(section["title"])
        f.write("\n")
        f.write("+" * len(section["title"]))
        f.write("\n")

        if not section["body"].startswith("\n"):
            f.write("\n")
        f.write(section["body"])
        if not section["body"].endswith("\n\n"):
            f.write("\n")

        if "modes" in schema:
            f.write("Supported modes: %s.\n\n" % ", ".join(schema["modes"]).upper())

        f.write(("==========\n"
                 "Properties\n"
                 "==========\n\n")
        )

        common_properties = [p + "_" for p in common_schema]
        common_properties.sort()
        f.write("Common properties: %s.\n\n" % ", ".join(common_properties))

        type_properties = schema.get("type_properties")
        if type_properties:
            for prop_name in type_properties:
                p = type_properties[prop_name]
                f.write("{} : {}\n".format(prop_name, p["type"]))
                if p.get("description", ""):
                    f.write("  {}".format(p["description"]))
                if isinstance(p.get("options"), list):
                    if p.get("description"):
                        f.write(" Possible values are:\n\n")
                    else:
                        f.write("  Possible values are:\n\n")
                    for opt in p["options"]:
                        if isinstance(opt, list):
                            if len(opt) > 1:
                                f.write("  * {} - {}\n".format(opt[0], opt[1]))
                        else:
                            f.write("  * {}\n".format(opt))
                    f.write("\n")
                else:
                    f.write("\n\n")

                if "default" in p:
                    f.write("  Default value: {}.\n\n".format(p["default"]))


def extract_all(root, dst):
    schemas = {}
    sections = {}

    for path in Path(root).rglob("*"):
        if path.is_file():
            parse_file(path, sections, schemas)

    schemas_sorted = list(schemas.values())
    schemas_sorted.sort(key = lambda s: s["schema"].get("tags",[""])[0])

    open(Path(dst) / "_connectors.rst", "w").close()
    open(Path(dst) / "_filtras.rst", "w").close()

    for schema in schemas_sorted:
        schema_id = schema["id"]
        if schema_id in sections:
            print("Generating section for", schema_id)
            if schema_id.endswith("connector"):
                generate_rst(Path(dst) / "_connectors.rst", sections[schema_id], schema["schema"], schemas.get(schema["ext_schema_id"], {}).get("schema"))
            elif schema_id.endswith("filtra"):
                generate_rst(Path(dst) / "_filtras.rst", sections[schema_id], schema["schema"], schemas.get(schema["ext_schema_id"], {}).get("schema"))


if __name__ == "__main__":
    extract_all(sys.argv[1], sys.argv[2])
