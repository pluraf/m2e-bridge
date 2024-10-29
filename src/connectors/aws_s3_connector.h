#ifndef __M2E_BRIDGE_AWS_S3_CONNECTOR_H__
#define __M2E_BRIDGE_AWS_S3_CONNECTOR_H__

#include <string>
#include <iostream>
#include <sstream> 
#include <stdexcept>
#include <regex>

#include <iomanip>

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/CreateBucketRequest.h>

#include "connector.h"
#include "database.h"


class S3Connector: public Connector {
private:
    Aws::String bucket_name_;
    std::string authbundle_id_;
    std::string access_key_;
    std::string secret_key_;
    std::string object_name_template_;

    bool is_object_template_ {false};
    bool delete_after_processing_ {false};

    Aws::S3::S3Client client_;
    Aws::Auth::AWSCredentials credentials_;

    void parse_authbundle(){
        Database db;
        AuthBundle ab;
        bool res = db.retrieve_authbundle(authbundle_id_, ab);
        if(res){
            if(ab.connector_type != ConnectorType::AWS_S3){
                throw std::runtime_error("Incompatiable authbundle connector type\n");
            }
            if(ab.auth_type != AuthType::PASSWORD){
                throw std::runtime_error("Incompatiable authbundle auth type\n");
            }
            access_key_ = ab.username;
            secret_key_ = ab.password;
        }else{
            throw std::runtime_error("Not able to retreive bundle\n");
        }
    }

public:
    S3Connector(std::string pipeid, ConnectorMode mode, json const & json_descr):
            Connector(pipeid, mode, json_descr){
        ConnectorMode mode_ = mode;
        try{
            authbundle_id_ = json_descr.at("authbundle_id").get<std::string>();
            parse_authbundle();
        }catch(json::exception){
            throw std::runtime_error("authbundle_id cannot be null for s3 connector\n");
        }

        try{
            bucket_name_ = json_descr.at("bucket_name").get<std::string>();
        }catch(json::exception){
            throw std::runtime_error("Bucket name cannot be null for s3 connector\n");
        }

        try{
            object_name_template_ = json_descr.at("object_name").get<std::string>();
        }catch(json::exception){
            throw std::runtime_error("Object name cannot be null for s3 connector\n");
        }

        if(json_descr.contains("delete_after_processing")){
            delete_after_processing_ = json_descr.at("delete_after_processing").get<bool>();
        }

        Aws::Auth::AWSCredentials credentials_(access_key_.c_str(), secret_key_.c_str());
        auto provider = Aws::MakeShared<Aws::Auth::SimpleAWSCredentialsProvider>
                                        ("S3Connector", access_key_, secret_key_);

        Aws::Client::ClientConfiguration client_config;

        client_ = Aws::S3::S3Client(provider, nullptr, client_config);

        std::smatch match;
        std::regex pattern("\\{\\{(.*?)\\}\\}");
        is_object_template_ = std::regex_search(object_name_template_.cbegin(), 
                                                object_name_template_.cend(), 
                                                match, 
                                                pattern);
    }

    void connect()override{
        std::cout << "Connecting to AWS S3 bucket: " << bucket_name_ << std::endl;

        Aws::S3::Model::ListObjectsRequest request;
        request.SetBucket(bucket_name_);

        auto outcome = client_.ListObjects(request);
        if(!outcome.IsSuccess()){
            if(mode_ == ConnectorMode::IN){
                throw std::runtime_error("Bucket '" + bucket_name_ + "' does not exist!");
            }else{
                std::cerr << "Bucket not found. Attempting to create it..." << std::endl;

                Aws::S3::Model::CreateBucketRequest create_request;
                create_request.SetBucket(bucket_name_);

                auto create_outcome = client_.CreateBucket(create_request);
                if(!create_outcome.IsSuccess()){
                    std::cerr << "Failed to create bucket: " 
                            << create_outcome.GetError().GetMessage() << std::endl;
                    throw std::runtime_error("Error creating bucket: " + 
                                             create_outcome.GetError().GetMessage());
                }else{
                    std::cout << "Bucket created successfully." << std::endl;
                    outcome = client_.ListObjects(request);
                }
            }
        }else{
        std::cout << "Bucket exists. Proceeding with connection..." << std::endl;
        }

        if(outcome.IsSuccess()){
            std::cout << "Connected to bucket successfully: " << bucket_name_ << std::endl;
        }else{
            throw std::runtime_error("Unable to connect to AWS S3 bucket.");
        }
    }

    std::string generate_timestamp(){
        auto current_time = time(nullptr);
        tm time_info{};

        if(localtime_r(&current_time, &time_info) == nullptr){
            throw std::runtime_error("localtime_r() failed");
        }
        std::ostringstream oss;
        oss << std::put_time(&time_info, "%Y%m%d_%H%M");
        std::string timestamp(oss.str());
        return timestamp;
    }

    std::string derive_object_name(Message & msg){
        std::string file_name = object_name_template_;
        std::regex pattern("\\{\\{(.*?)\\}\\}");

        auto pos = file_name.cbegin();
        std::smatch match;

        while(regex_search(pos, file_name.cend(), match, pattern)){
            std::string ve = match[1].str();
            if(ve == "topic"){
                std::string vvalue = msg.get_topic(); 

                vvalue.erase(remove(vvalue.begin(), vvalue.end(), '/'), vvalue.end());

                unsigned int i = (pos - file_name.cbegin());
                file_name.replace(i + match.position(), match.length(), vvalue);
                pos = file_name.cbegin() + i + match.position() + vvalue.size(); 
            }else if(ve == "timestamp"){
                std::string timestamp = generate_timestamp();

                unsigned int i = (pos - file_name.cbegin());
                file_name.replace(i + match.position(), match.length(), timestamp);
                pos = file_name.cbegin() + i + match.position() + timestamp.size();
            }else{
                try{
                    json const& payload = msg.get_raw();
                    std::string vvalue = payload.at(ve);

                    unsigned int i = (pos - file_name.cbegin());
                    file_name.replace(i + match.position(), match.length(), vvalue);
                    pos = file_name.cbegin() + i + match.position() + vvalue.size();
                }catch(json::exception& e){
                    throw std::runtime_error(fmt::format("Template variable {} not found in the payload!", ve));
                }
            }
        }

        return file_name;
    }

    void send(Message & msg)override{
        std::string file_name = derive_object_name(msg);
        std::string file_content = msg.get_raw();

        Aws::S3::Model::PutObjectRequest request;
        request.SetBucket(bucket_name_);
        request.SetKey(file_name);

        auto input_data = Aws::MakeShared<Aws::StringStream>("", file_content);
        request.SetBody(input_data);

        auto outcome = client_.PutObject(request);
        if (!outcome.IsSuccess()) {
            throw std::runtime_error("Failed to upload file to S3: " + outcome.GetError().GetMessage());
        }

        std::cout << "File uploaded to AWS S3 as object: " << file_name << std::endl;
    }

    Message receive()override {
        Aws::S3::Model::GetObjectRequest request;
        request.SetBucket(bucket_name_);
        request.SetKey(object_name_template_);

        auto outcome = client_.GetObject(request);
        if (!outcome.IsSuccess()) {
            throw std::runtime_error("Failed to read file from S3: " + outcome.GetError().GetMessage());
        }

        std::ostringstream stream;
        stream << outcome.GetResultWithOwnership().GetBody().rdbuf();
        std::string file_content = stream.str();

        std::cout << "Message received from AWS S3: " << file_content << std::endl;

        if(delete_after_processing_){
            Aws::S3::Model::DeleteObjectRequest delete_request;
            delete_request.SetBucket(bucket_name_);
            delete_request.SetKey(object_name_template_);

            auto delete_outcome = client_.DeleteObject(delete_request);
            if(!delete_outcome.IsSuccess()){
                std::cerr << "Failed to delete object after processing: "
                          << delete_outcome.GetError().GetMessage() << std::endl;
            } else{
                std::cout << "Object deleted after processing." << std::endl;
            }
        }

        return Message(file_content, object_name_template_);
    }
};


#endif