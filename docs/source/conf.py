# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'M2E-Bridge'
copyright = '2024-2026, <a href="https://pluraf.com">Pluraf Embedded AB</a>'
author = 'Konstantin Tyurin'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ["breathe"]

templates_path = ['_templates']

exclude_patterns = ["_*.rst"]

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'alabaster'
html_theme_options = {
    'page_width': '1100px',
    'sidebar_width': '200px',
    'fixed_sidebar': True,
    'extra_nav_links': {
        "GitHub": "https://github.com/pluraf",
    }
}

html_static_path = ['_static']
html_css_files = ['custom.css']

breathe_projects = {"m2e-bridge": "../../xml/"}
breathe_default_project = "m2e-bridge"