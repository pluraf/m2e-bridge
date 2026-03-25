.. _AWS S3 Connector:
.. _AWS S3 Connectors:

++++++++++++++++
AWS S3 Connector
++++++++++++++++

.. _Simple Storage Service: https://aws.amazon.com/s3/

Connects to Amazon Web Services `Simple Storage Service`_ (S3)::

    {
      "type": "aws_s3",
      "authbundle_id": "<authbundle_id>",
      "bucket_name": "<bucket_name>",
      "object_name": "<object_name>"
    }

Supported modes: IN, OUT.

==========
Properties
==========

Common properties: authbundle_id_, type_.

bucket_name : string
  Cloud Storage bucket name.

object_name : string
  Object name inside the bucket.

delete_received : boolean
  Deletes the object after downloading.

  Default value: False.

.. _Azure Service Bus Connector:
.. _Azure Service Bus Connectors:

+++++++++++++++++++++++++++
Azure Service Bus Connector
+++++++++++++++++++++++++++

.. _Service Bus: https://learn.microsoft.com/en-us/connectors/servicebus

Connects to the Microsoft Azure `Service Bus`_::

    {
      "type":"azure_sbc",
      "authbundle_id": "<authbundle_id>",
      "entity_path": "<entity_path>",
      "expire_in": <seconds>,
      "is_topic": "<is_topic>",
      "subscription_name": "<subscription_name>",
      "delete_after_processing": "<delete_after_processing>"
    }

Supported modes: IN, OUT.

==========
Properties
==========

Common properties: authbundle_id_, type_.

entity_path : string
  Entity path.

expire_in : integer
  Validity of sas token.

  Default value: 3600.

is_topic : boolean


subscription_name : string


delete_after_processing : boolean


.. _SQLite Connector:
.. _SQLite Connectors:

++++++++++++++++
SQLite Connector
++++++++++++++++

.. _SQLite: https://sqlite.org

Connects to a `SQLite`_ database::

    {
        "type": "sqlite",
        "db_path": "<path>",
        "table": "<table>",
        "columns": [
            "<column1>",
            "<colimn2>"
        ],
        "values": [
            "<value1>",
            "<value1>"
        ]
    }

Supported modes: OUT.

==========
Properties
==========

Common properties: authbundle_id_, type_.

db_path : string
  Database file.

table : string
  Database table.

columns : array
  Columns where values will be inserted.

values : array
  Values to be inserted into the table.

.. _PostgreSQL Connector:
.. _PostgreSQL Connectors:

++++++++++++++++++++
PostgreSQL Connector
++++++++++++++++++++

.. _PostgreSQL: https://www.postgresql.org

Connects to a `PostgreSQL`_ database::

    {
        "type": "postgresql",
        "db_name": "<database>",
        "db_host": "<IP>",
        "db_port": <PORT>,
        "authbundle_id": "<authbundle_id>",
        "table": "<table>",
        "columns": [
            "<column1>",
            "<colimn2>"
        ],
        "values": [
            "<value1>",
            "<value1>"
        ]
    }

Supported modes: OUT.

==========
Properties
==========

Common properties: authbundle_id_, type_.

db_host : string
  Database server host.

db_port : string
  Database server port.

db_name : string
  Database name.

table : string
  Database table.

columns : array
  Columns where values will be inserted.

values : array
  Values to be inserted into the table.

.. _Email Connector:
.. _Email Connectors:

+++++++++++++++
Email Connector
+++++++++++++++

Sends email through SMTP or IMAP server::

    {
      "type": "email",
      "smtp_server": "<smtp_server>",
      "smtp_port": <smtp_port>,
      "imap_server": "<imap_server>",
      "imap_port": <imap_port>,
      "subject": "<email subject>",
      "address": "<to address>"
    }

Supported modes: OUT.

==========
Properties
==========

Common properties: authbundle_id_, type_.

address : string


smtp_server : string


smtp_port : integer


  Default value: 587.

imap_server : string


imap_port : integer


  Default value: 993.

subject : string


.. _GCP Pub/Sub Connector:
.. _GCP Pub/Sub Connectors:

+++++++++++++++++++++
GCP Pub/Sub Connector
+++++++++++++++++++++

.. _Pub/Sub service: https://cloud.google.com/pubsub/docs/overview

Connects to the Google Cloud `Pub/Sub service`_::

    {
      "type":"gcp_pubsub",
      "authbundle_id":"my_authbundle_id",
      "project_id":"my_project_id",
      "topic_id":"my_topic_id",
      "attributes":{
        "deviceId":"{{topic[2]}}",
        "projectId": "G-NODE"
      }
    }

Supported modes: IN, OUT.

==========
Properties
==========

Common properties: authbundle_id_, type_.

project_id : string
  GCP project identifier.

subscription_id : string
  Subscription that the connector-in subscribes to. If the subscription does not exist, it is created. The connector-out ignores this property.

topic_id : string
  PubSub topic. The connector-in publishes to this topic. The connector-out creates a subscription to this topic if *subscription_id* is not specified.

attributes : object
  Map of Pub/Sub message attributes.

.. _GCP Cloud Storage Connector:
.. _GCP Cloud Storage Connectors:

+++++++++++++++++++++++++++
GCP Cloud Storage Connector
+++++++++++++++++++++++++++

.. _Cloud Storage service: https://cloud.google.com/storage/docs/introduction

Connects to the Google Cloud `Cloud Storage service`_::

    {
      "type":"gcp_storage",
      "authbundle_id":"my_service_key",
      "project_id":"my_project_id",
      "bucket_name":"my_bucket_name",
      "object_name": "my_object_name"
    }

Supported modes: IN, OUT.

==========
Properties
==========

Common properties: authbundle_id_, type_.

project_id : string
  The GCP project identifier.

bucket_name : string
  The bucket name to which the connector-out uploads messages or from which the connector-in downloads messages. If the bucket does not exist, the connector-out creates it, whereas the connector-in does not.

object_name : string
  The name of the bucket object where the message will be stored. If it is a plain string, it will be overwritten with each new message.

delete_after_processing : boolean
  Delete object from the bucket.

.. _Genrator Connector:
.. _Genrator Connectors:

++++++++++++++++++
Genrator Connector
++++++++++++++++++

Connects to internal message generator::

    {
      "type": "generator",
      "period": <seconds>,
      "payload": {}
    }

Supported modes: OUT.

==========
Properties
==========

Common properties: authbundle_id_, type_.

period : integer


payload : object


.. _HTTP Connector:
.. _HTTP Connectors:

++++++++++++++
HTTP Connector
++++++++++++++

Connects to HTTP server::

    {
      "type": "http",
      "url": "<url>",
      "method": "<method>",
      "header": {
        "<name1>": "<value1>",
        "<name2>": "<value2>"
      },
      "payload": {
        "<key1>": "<val1>",
        "<key2>": "<val2>"
      },
      "polling_period": <seconds>,
      "https_verify_cert": true
    }

Supported modes: IN, OUT.

==========
Properties
==========

Common properties: authbundle_id_, type_.

url : string
  URL of HTTP server.

https_verify_cert : boolean
  Verify HTTP server certificate.

  Default value: True.

method : string
  HTTP method (GET, POST, PUT, etc.).

header : object
  Map of HTTP headers.

payload : string|object|array
  Payload of HTTP request.

polling_period : integer
  Polling period when connector is in IN mode.

.. _MQTT Connector:
.. _MQTT Connectors:

++++++++++++++
MQTT Connector
++++++++++++++

.. _MQTT: https://mqtt.org/

Connects to an `MQTT`_ Broker::

    {
      "type": "mqtt",
      "topic": "/topic/+/event",
      "server": "mqtt://mqtt.iotplan.io:1883"
    }

Supported modes: IN, OUT.

==========
Properties
==========

Common properties: authbundle_id_, type_.

server : string
  URL of the MQTT Broker.

  Default value: tcp://mqtt.iotplan.io:1883.

version : string
  MQTT protocol version. Possible values are:

  * 5
  * 3.11

  Default value: 5.

topic : string
  The MQTT topic to subscribe to for connector-in or to publish to for connector-out.

client_id : string
  The name of the bucket object where the message will be stored. If it isa plain string, it will be overwritten with each new message.

retry_attempts : integer
  Number of connection attempts.

  Default value: 10.

qos : integer
  MQTT Quality of Service (QoS) level. Possible values are:

  * 0 - At Most Once
  * 1 - At Least Once
  * 2 - Exactly Once

  Default value: 1.

ca_certificate : string
  CA certificate (should be uploaded in advance).

verify_server_hostname : string
  Verifies that server certificate matches its hostname. Possible values are:

  * yes
  * no

  Default value: yes.

verify_server_certificate : string
  Verifies server certificate. Possible values are:

  * yes
  * no

  Default value: yes.

.. _Internal Queue Connector:
.. _Internal Queue Connectors:

++++++++++++++++++++++++
Internal Queue Connector
++++++++++++++++++++++++

Connects to internal message queue::

    {
      "type": "queue",
      "name": "<queue name>",
      "buffer_size": <number>
    }

Supported modes: IN, OUT.

==========
Properties
==========

Common properties: authbundle_id_, type_.

name : string
  Internal queue name.

buffer_size : integer
  Buffer size for messages.

  Default value: 100.

.. _Slack Connector:
.. _Slack Connectors:

+++++++++++++++
Slack Connector
+++++++++++++++

.. _Slack: https://slack.com

Connects to `Slack`_::

    {
      "type":"slack",
      "authbundle_id":"<authbundle_id>"
    }

Slack webhook URL must be stored in Authbundle.

Supported modes: OUT.

==========
Properties
==========

Common properties: authbundle_id_, type_.

