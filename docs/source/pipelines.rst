.. _Pipeline:

#########
Pipelines
#########

Pipelines process data through a sequence of filtering and transforming blocks, known as Filtras_.
Data flows into and out of pipelines through Connectors_.
A single data unit traveling through the pipeline is called a *message*.

Each pipeline has a *connector-in* (input connector) and a *connector-out* (output connector),
along with an optional array of *filtras*.

Here is an example of a JSON configuration for the simplest pipeline, which simply receives
MQTT messages and sends them as-is to GCP Pub/Sub::

    "pipeline_1": {
      "connector_in": {
        "type": "mqtt",
        "topic": "/topic/+/event"
      },
      "connector_out":{
        "type":"gcp_pubsub",
        "authbundle_id":"my_authbundle_id",
        "project_id":"my_project_id",
        "topic_id":"my_gcp_topic_id"
      }
    }

This configuration is an extended version of the first example. In addition to transferring
messages from MQTT to Pub/Sub, it inspects each message, allowing through only those that have
a *temp* value above *5.4* and do not contain any *LOG* substrings::

    "pipeline_1":{
      "connector_in":{
        "type": "mqtt",
        "topic": "/topic/+/event"
      },
      "connector_out":{
        "type":"gcp_pubsub",
        "authbundle_id":"my_authbundle_id",
        "project_id":"my_project_id",
        "topic_id":"my_gcp_topic_id"
      },
      "filtras":[
        {
          "type": "comparator",
          "operator": "gt",
          "decoder": "json",
          "value_key": "temp",
          "comparand": 5.4
        },
        {
          "type": "finder",
          "operator": "contain",
          "logical_negation": true,
          "string": "LOG"
        }
      ]
    }

By default, each pipeline processes data sequentially. Messages move through each filtra until
they reach the connector-out or are rejected by one of the filtras. Complex flow control,
including if/else logic and loops, can be implemented using filtra `goto properties`_,
which allow messages to move forward or backward in the sequence of filtras.

**********
Connectors
**********

Connectors handle the input and output of messages in a pipeline. Most connectors
support *in* and *out* modes. A **connector-in** is defined under the *connector_in* key
in the pipeline config, while the *connector_out* key defines the **connector-out**.

++++++++++++++++++++++
Authentication bundles
++++++++++++++++++++++

Authentication bundles, or *authbundles*, store authentication data for connecting to
external services. The required data varies based on the service type. Currently, supported types
include:

* *gcp* - GCP service key;
* *aws* - AWS access key;
* *mqtt311*, *mqtt50* - MQTT authentication based on CONNECT message
  Supported options are:

    * *Username/Password* - Plain string literals for Username and Password;
    * *JWT* - Password serves as a JWT token;

  **Note**: In MQTT v3.11, *Username* cannot be empty, unlike in v5.0.

++++++++++++++++++++
Connector Properties
++++++++++++++++++++

Each connector must define the *type* property. While most other properties are specific
to each connector type, a few are common across many connectors:

.. _authbundle_id:

authbundle_id : string
  Specifies the identifier of an authentication bundle to be used in the authentication process
  with the service the connector connects to.

.. _type:

type : string
  Specifies the connector type. Valid options include:

  * mqtt
  * gcp_pubsub
  * gcp_bucket
  * aws_s3,
  * email
  * queue
  * slack

A connector may require different properties depending on whether it is in *in* or *out* mode.


.. include:: _connectors.rst


.. _Filtra:
.. _Filtras:

*******
Filtras
*******

A filtra is a message-processing unit that functions as either a *filter* or a *transformer*.

*Filters* are units that do not modify the original message received through the connector-in.
Their primary purpose is to filter out messages that do not meet specific criteria.

In contrast, *transformers* are units that can modify the original message,
though some transformers may pass certain messages unchanged.

Pipelines can include any number of filtras, which can be linked to create complex data-processing
flows using `goto properties`_.

+++++++++++++++++
Filtra Properties
+++++++++++++++++

All filtras require a **type** property and may include other optional properties. Common
properties for all filtras are described here, while type-specific properties are covered in their
respective sections.

=================
Common Properties
=================

.. _msg_format:

msg_format : string
  Specifies the msg_format used for decoding/encoding the message payload. Supported values:

  * json
  * corb

.. _goto properties:
.. _goto_accepted:

goto_accepted : string
  Specifies the name of the next filtra in the pipeline if the current filtra allows
  the message to pass.

.. _goto_rejected:

goto_rejected : string
  Specifies the name of the next filtra in the pipeline if the message is rejected
  by the current filtra.

.. _goto:

goto : string
  Equivalent to goto_accepted_.

.. _name:

name : string
  Specifies a name for the filtra to be used in *goto* properties.
  The names *self* and *out* are reserved.

.. _logical_negation:

logical_negation : boolean
  Specifies that logical negation is applied to the result, meaning an accepted message
  becomes rejected, and a rejected message becomes accepted.

.. _metadata:

metadata : dictionary
  Specifies a dictionary of attributes that can be used for various purposes, such as
  `Dynamic substitutions`_ in property values.

.. _queues:

queues : array of strings
  Specifies the names of internal queues used for message interchange between pipelines.


.. include:: _filtras.rst


*********************
Dynamic substitutions
*********************

Dynamic substitutions are supported in certain places. Placeholders for dynamic substitutions are
defined using double curly braces: ``{{dynamic_value}}``. ::

    "filtras":[
      {
        "type": "nop",
        "metadata":{
          "topic": "/big"
        },
      }
    ],
    "connector_out": {
      "type": "mqtt",
      "topic": "{{topic}}",
      "server": "mqtt://mqtt.iotplan.io:1883"
    }

In the example above, the `Nop Filtra`_ defines *metadata* that is used by the *connector-out*
to replace ``{{topic}}`` with the actual value from the *metadata* dictionary. As a result,
the *connector-out* sends each message to the topic ``/big``.

It is not always necessary to explicitly define the *metadata* property, as some *metadata*
key-value pairs are automatically derived::

    "connector_in": {
      "type": "mqtt",
      "topic": "/topic/+/event",
      "server": "mqtt://mqtt.iotplan.io:1883",
    },
    "connector_out":{
      "type":"gcp_pubsub",
      "authbundle_id":"my_authbundle_id",
      "project_id":"my_project_id",
      "topic_id":"my_topic_id",
      "attributes":{
        "deviceId":"{{topic[2]}}",
      }
    }

In the example above, the ``topic`` in the *connector-out* refers to the topic that
the *connector-in* is subscribed to. This also demonstrates indexing in dynamic substitutions.
The *connector-in* subscribes to an MQTT topic composed of multiple levels,
and dynamic substitution with indexing allows extracting a specific level of interest from
the topic to replace the placeholder ``{{topic[2]}}``. In this case, the *connector-in*
is subscribed to a topic with a wildcard character. As a result, the value of ``{{topic[2]}}``
varies from message to message, depending on the actual topic to which the message was published.