function(add_target TARGET_NAME)

    add_executable(${TARGET_NAME}
        ${CORE_SOURCES}
        ${OPTIONAL_SOURCES}
    )

    target_compile_definitions(${TARGET_NAME} PRIVATE
        $<$<BOOL:${WITH_GCP_CLOUD_STORAGE_CONNECTOR}>:WITH_GCP_CLOUD_STORAGE_CONNECTOR>
        $<$<BOOL:${WITH_GCP_PUBSUB_CONNECTOR}>:WITH_GCP_PUBSUB_CONNECTOR>

        $<$<BOOL:${WITH_AWS_S3_CONNECTOR}>:WITH_AWS_S3_CONNECTOR>
        $<$<BOOL:${WITH_SQLITE_CONNECTOR}>:WITH_SQLITE_CONNECTOR>
        $<$<BOOL:${WITH_POSTGRESQL_CONNECTOR}>:WITH_POSTGRESQL_CONNECTOR>
        $<$<BOOL:${WITH_MODBUS_CONNECTOR}>:WITH_MODBUS_CONNECTOR>
        $<$<BOOL:${WITH_MQTT_CONNECTOR}>:WITH_MQTT_CONNECTOR>
        $<$<BOOL:${WITH_EMAIL_CONNECTOR}>:WITH_EMAIL_CONNECTOR>
        $<$<BOOL:${WITH_HTTP_CONNECTOR}>:WITH_HTTP_CONNECTOR>
        $<$<BOOL:${WITH_SLACK_CONNECTOR}>:WITH_SLACK_CONNECTOR>
        $<$<BOOL:${WITH_AZURE_SERVICE_BUS_CONNECTOR}>:WITH_AZURE_SERVICE_BUS_CONNECTOR>

        $<$<BOOL:${WITH_HTTP_LISTENER}>:WITH_HTTP_LISTENER>

        $<$<BOOL:${WITH_IMAGE_HANDLERS}>:WITH_IMAGE_HANDLERS>

        $<$<BOOL:${WITH_LUA}>:WITH_LUA>

        $<$<BOOL:${WITH_ZEROMQ_API}>:WITH_ZEROMQ_API>
        $<$<BOOL:${WITH_HTTP_API}>:WITH_HTTP_API>

        $<$<BOOL:${_WITH_CIVETWEB}>:_WITH_CIVETWEB>
        $<$<BOOL:${_WITH_CURL}>:_WITH_CURL>
        $<$<BOOL:${_WITH_OPENSSL}>:_WITH_OPENSSL>
        $<$<BOOL:${_WITH_OPENCV}>:_WITH_OPENCV>
    )

    target_compile_definitions(${TARGET_NAME} PRIVATE JWT_DISABLE_PICOJSON)
    target_link_libraries(${TARGET_NAME} PRIVATE nlohmann_json::nlohmann_json)

    ##########################################################################

    if (_WITH_CIVETWEB )
        target_link_libraries(${TARGET_NAME} PRIVATE civetweb::civetweb-cpp)
    endif()

    if( _WITH_CURL )
        target_link_libraries(${TARGET_NAME} PRIVATE CURL::libcurl)
    endif()

    if( WITH_GCP_PUBSUB_CONNECTOR )
        target_link_libraries(${TARGET_NAME} PRIVATE google-cloud-cpp::pubsub)
    endif()

    if( WITH_GCP_CLOUD_STORAGE_CONNECTOR )
        target_link_libraries(${TARGET_NAME} PRIVATE google-cloud-cpp::storage)
    endif()

    if( WITH_SQLITE_CONNECTOR )
        target_link_libraries(${TARGET_NAME} PRIVATE SQLite3::SQLite3)
    endif()

    if( WITH_AWS_S3_CONNECTOR )
        target_link_libraries(${TARGET_NAME} PRIVATE aws-cpp-sdk-core)
        target_link_libraries(${TARGET_NAME} PRIVATE aws-cpp-sdk-s3)
    endif()

    if( WITH_MODBUS_CONNECTOR )
        target_link_libraries(${TARGET_NAME} PRIVATE
            ${_REFLECTION}
            ${_GRPC_GRPCPP}
            ${_PROTOBUF_LIBPROTOBUF}
        )
    endif()

    if( WITH_POSTGRESQL_CONNECTOR )
        target_link_libraries(${TARGET_NAME} PRIVATE libpqxx::pqxx)
    endif()

    if( WITH_MQTT_CONNECTOR )
        target_link_libraries(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/externals/paho-mqtt-c/build/src/libpaho-mqtt3as.a")
        target_link_libraries(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/externals/paho-mqtt-c/build/src/libpaho-mqtt3cs.a")
    endif()

    if( _WITH_OPENSSL )
        target_link_libraries(${TARGET_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    endif()

    if( _WITH_OPENCV )
        target_link_libraries(${TARGET_NAME} PRIVATE opencv_core opencv_imgproc opencv_imgcodecs)
    endif()

    if( WITH_ZEROMQ_API )
        target_link_libraries(${TARGET_NAME} PRIVATE ${ZeroMQ_LIBRARIES})
    endif()

    if( WITH_LUA )
        target_link_libraries(${TARGET_NAME} PRIVATE ${LUA_LIBRARIES})
        target_link_libraries(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/external/lua-cjson/build/cjson.a")
    endif()

    target_include_directories(${TARGET_NAME} PRIVATE ${JWT_CPP_INCLUDE_DIRS})
    target_link_libraries(${TARGET_NAME} PRIVATE fmt::fmt)
    target_link_libraries(${TARGET_NAME} PRIVATE Poco::Foundation)
    target_link_libraries(${TARGET_NAME} PRIVATE Poco::Foundation)
    target_link_libraries(${TARGET_NAME} PRIVATE cbor)

endfunction()