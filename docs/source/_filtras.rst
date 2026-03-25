.. _Limiter Filtra:
.. _Limiter Filtras:

++++++++++++++
Limiter Filtra
++++++++++++++

Rejects messages that exceed the specified size::

      {
          "type": "limiter",
          "size": <bytes>
      }

==========
Properties
==========

Common properties: goto_, goto_accepted_, goto_rejected_, logical_negation_, metadata_, name_, queues_.

.. _Builder Filtra:
.. _Builder Filtras:

++++++++++++++
Builder Filtra
++++++++++++++

Creates a new message::

    {
      "type": "builder",
      "encoder": "json",
      "payload": {
        "<key1>": "<value1>"
      }
      "extra": {
        "<key1>": "<value1>"
      }
    }

==========
Properties
==========

Common properties: goto_, goto_accepted_, goto_rejected_, logical_negation_, metadata_, name_, queues_.

encoder : string
  Possible values are:

  * json

  Default value: json.

payload : object


extra : object


.. _Comparator Filtra:
.. _Comparator Filtras:

+++++++++++++++++
Comparator Filtra
+++++++++++++++++

Creates a new message::

    {
      "type": "comparator",
      "operator": "gt",
      "value_key": "temp",
      "comparand": 5.4
    }

==========
Properties
==========

Common properties: goto_, goto_accepted_, goto_rejected_, logical_negation_, metadata_, name_, queues_.

operator : string
  Comparison operator. Possible values are:

  * eq - equal
  * gt - greater than
  * gte - greater than or equal
  * lt - less than
  * lte - less than or equal

value_key : string
  Key in the payload whose value is compared with the comparand.

comparand : float
  Numerical constant to be compared with a value in the payload.

.. _Lua Converter Filtra:
.. _Lua Converter Filtras:

++++++++++++++++++++
Lua Converter Filtra
++++++++++++++++++++

Converts messages with a custom Lua script::

    {
      "type": "lua_converter",
      "converter_id": "<converter_id>"
    }

==========
Properties
==========

Common properties: goto_, goto_accepted_, goto_rejected_, logical_negation_, metadata_, name_, queues_.

converter_id : string


.. _Eraser Filtra:
.. _Eraser Filtras:

+++++++++++++
Eraser Filtra
+++++++++++++

Removes specified keys from the payload::

    {
      "type": "eraser",
      "keys": ["<key1>", "<key2>"]
    }

==========
Properties
==========

Common properties: goto_, goto_accepted_, goto_rejected_, logical_negation_, metadata_, name_, queues_.

keys : array
  Keys to be removed from the message payload.

.. _Finder Filtra:
.. _Finder Filtras:

+++++++++++++
Finder Filtra
+++++++++++++

Searches for a specific part in the message based on the finder configuration.
Three variants are supported:

.. __:

1. Searches for text within the payload or searches for the payload within the text.
   Properties *text* and *operator* must be specified::

      {
          "type": "finder",
          "operator": "contain",
          "text": "LOG"
      }

2. Searches for specified keys in the decoded payload. A property *keys* must be specified.

   ::

      {
          "type": "finder",
          "keys": ["key1", "keyN"]
      }

3. Performs the same type of search as in `Variant 1`__, but instead of analyzing
   the entire payload, it examines the value of a specified key in
   the decoded payload. Properties *text*, *operator*, *value_key* must be specified::

      {
          "type": "finder",
          "operator": "contain",
          "text": "LOG",
          "value_key": "key1"
      }

==========
Properties
==========

Common properties: goto_, goto_accepted_, goto_rejected_, logical_negation_, metadata_, name_, queues_.

operator : string
  Method for searching for a substring. Possible values are:

  * contain - searches for *text* within the payload
  * contained - searches for the payload within *text*
  * match - checks if *text* exactly matches the payload

  Default value: match.

text : string
  Specifies a string that can either be searched as a substring within the payload or used to search for the payload as a substring.

keys : array
  Specifies the keys in the decoded payload dictionary. All keys must be present in the payload; otherwise, the message is rejected.

value_key : string
  Specifies a key in the decoded payload whose value is used for the text search.

decoder : string
  Decoder for decoding the message. Possible values are:

  * json
  * raw

  Default value: raw.

.. _Image Filtra:
.. _Image Filtras:

++++++++++++
Image Filtra
++++++++++++

Creates a new message::

    {
      "type": "image",
      "operation": "<operation>"
    }

==========
Properties
==========

Common properties: goto_, goto_accepted_, goto_rejected_, logical_negation_, metadata_, name_, queues_.

operation : string
  Image transformation operation. Possible values are:

  * resize

extra : object


.. _NOP Filtra:
.. _NOP Filtras:

++++++++++
NOP Filtra
++++++++++

Performs no action; however, it can be used, for example, to attach metadata
or direct messages to internal queues::

      {
          "type": "nop"
      }

==========
Properties
==========

Common properties: goto_, goto_accepted_, goto_rejected_, logical_negation_, metadata_, name_, queues_.

.. _Splitter Filtra:
.. _Splitter Filtras:

+++++++++++++++
Splitter Filtra
+++++++++++++++

Splits messages that exceed a specified size into chunks. If the message size is below *chunk_size*,
it passes through the Splitter unmodified; otherwise, it is split into chunks of
*chunk_size* bytes (with the last chunk possibly smaller).
Each chunk is sent as a separate message. It is the receiver's responsibility to reconstruct
the original message::

      {
          "type": "splitter",
          "chunk_size": <bytes>
      }

Each chunk is encoded as a CBOR array containing the following elements:

* ____SPL - chunk marker
* Message ID - unique identifier for the original message (generated by the Splitter)
* Original Message Size - total size of the original message
* Chunk Number - sequential number of the chunk
* Array of Bytes - portion of the original message, with a length of *chunk_size* or smaller

==========
Properties
==========

Common properties: goto_, goto_accepted_, goto_rejected_, logical_negation_, metadata_, name_, queues_.

chunk_size : integer
  Maximum size of each message chunk in bytes.

.. _Throttle Filtra:
.. _Throttle Filtras:

+++++++++++++++
Throttle Filtra
+++++++++++++++

Limits messages rate::

    {
      "type": "throttle",
      "rate": <messages per second>
    }

==========
Properties
==========

Common properties: goto_, goto_accepted_, goto_rejected_, logical_negation_, metadata_, name_, queues_.

rate : integer
  Messages maximum rate, after which they start to be rejected.

