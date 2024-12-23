/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2024 Pluraf Embedded AB <code@pluraf.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to
do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
*/


#ifndef __M2E_BRIDGE_GCP_BUCKET_CONNECTOR_H__
#define __M2E_BRIDGE_GCP_BUCKET_CONNECTOR_H__


#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <regex>

#include <iomanip>

#include "google/cloud/storage/client.h"
#include "google/cloud/status.h"
#include "google/cloud/options.h"
#include "google/cloud/credentials.h"

#include "connector.h"
#include "database.h"


namespace gcloud = ::google::cloud;


class CloudStorageConnector: public Connector {
private:
    std::string project_id_;
    std::string bucket_name_;
    gcloud::storage::Client client_;
    std::string service_key_data_;
    std::string authbundle_id_;
    std::string object_name_template_;
    bool is_object_template_ {false};
    bool delete_after_processing_ {false};

    gcloud::Options auth_options_;

    void parse_authbundle(){
        Database db;
        AuthBundle ab;
        bool res = db.retrieve_authbundle(authbundle_id_, ab);
        if(res){
            if(ab.service_type != ServiceType::GCP){
                throw std::runtime_error("Incompatiable authbundle service type");
            }
            if(ab.auth_type != AuthType::SERVICE_KEY){
                throw std::runtime_error("Incompatiable authbundle auth type");
            }
            service_key_data_ = ab.keydata;
        }else{
            throw std::runtime_error("Not able to retreive bundle");
        }
    }

public:
    CloudStorageConnector(std::string pipeid, ConnectorMode mode, json const & json_descr):
            Connector(pipeid, mode, json_descr){
        ConnectorMode mode_ = mode;
        try{
            authbundle_id_ = json_descr.at("authbundle_id").get<std::string>();
            parse_authbundle();
        }catch(json::exception){
            throw std::runtime_error("authbundle_id cannot be null for bucket connector");
        }

        try{
            project_id_ = json_descr.at("project_id").get<string>();
        }catch(json::exception){
            throw std::runtime_error("Project ID cannot be null for bucket connector");
        }

        try{
            bucket_name_ = json_descr.at("bucket_name").get<std::string>();
        }catch(json::exception){
            throw std::runtime_error("Bucket name cannot be null for bucket connector");
        }

        try{
            object_name_template_ = json_descr.at("object_name").get<std::string>();
        }catch(json::exception){
            throw std::runtime_error("Object name cannot be null for bucket connector");
        }

        if(json_descr.contains("delete_after_processing")){
            delete_after_processing_ = json_descr.at("delete_after_processing").get<bool>();
        }

        auth_options_ = gcloud::Options{}
                        .set<gcloud::UnifiedCredentialsOption>(
                            gcloud::MakeServiceAccountCredentials(service_key_data_))
                        .set<gcloud::UserProjectOption>(project_id_);

        client_ = gcloud::storage::Client(auth_options_);

        std::smatch match;
        std::regex pattern("\\{\\{(.*?)\\}\\}");
        is_object_template_ = std::regex_search(object_name_template_.cbegin(),
                                                object_name_template_.cend(),
                                                match,
                                                pattern);
    }

    void connect()override{
        std::cout << "Connecting to GCP Bucket: " << bucket_name_ << std::endl;

        auto metadata = client_.GetBucketMetadata(bucket_name_);
        if(!metadata){
            if(mode_ == ConnectorMode::IN) {
                throw std::runtime_error(fmt::format("Bucket '{}' does not exist!", bucket_name_));
            }
            std::cerr << ("Bucket not found. Attempting to create it...") << std::endl;

            auto create_result = client_.CreateBucketForProject(
                                 bucket_name_, project_id_, gcloud::storage::BucketMetadata());

            if(!create_result) {
                std::cerr << "Failed to create bucket: " << create_result.status().message() << std::endl;
                throw std::runtime_error(fmt::format("Error creating bucket: {}", create_result.status().message()));
            }else {
                std::cout << "Bucket created successfully." << std::endl;
                metadata = client_.GetBucketMetadata(bucket_name_);
            }
        }else{
            std::cout << "Bucket exists. Proceeding with connection..." << std::endl;
        }

        if(metadata){
            std::cout << "Connected to bucket successfully: " << bucket_name_ << std::endl;
        }else{
            throw std::runtime_error("Unable to connect to GCP bucket.");
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

    std::string derive_object_name(MessageWrapper & msg_w){
        std::string file_name = object_name_template_;
        std::regex pattern("\\{\\{(.*?)\\}\\}");

        auto pos = file_name.cbegin();
        std::smatch match;

        while(regex_search(pos, file_name.cend(), match, pattern)){
            std::string ve = match[1].str();
            if(ve == "topic"){
                std::string vvalue = msg_w.msg().get_topic();

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
                    json const & metadata = msg_w.get_metadata();
                    std::string vvalue = metadata.at(ve);

                    unsigned int i = (pos - file_name.cbegin());
                    file_name.replace(i + match.position(), match.length(), vvalue);
                    pos = file_name.cbegin() + i + match.position() + vvalue.size();
                }catch(json::exception& e){
                    throw std::runtime_error(fmt::format("Template variable {} not found in the metadata!", ve));
                }
            }
        }

        return file_name;
    }

    void do_send(MessageWrapper & msg_w)override{
        std::string file_name = derive_object_name(msg_w);
        std::string file_content = msg_w.msg().get_raw();

        gcloud::storage::ObjectWriteStream stream = client_.WriteObject(bucket_name_, file_name);
        stream << file_content;
        stream.Close();

        if(!stream.metadata()){
            throw std::runtime_error("Error sending message to GCP Bucket.");
        }

        std::cout << "Message sent to GCP Bucket as object: " << file_name << std::endl;
    }

    Message do_receive()override{
        try{
            gcloud::storage::ObjectReadStream stream = client_.ReadObject(bucket_name_, object_name_template_);
            if(!stream){
                std::cerr << "Error reading message from GCP Bucket." << std::endl;
                throw std::runtime_error("Error reading message from GCP Bucket");
            }

            std::string file_content((std::istreambuf_iterator<char>(stream)),
                                      std::istreambuf_iterator<char>());

            std::cout << "Message received from GCP Bucket: " << file_content << std::endl;

            if(delete_after_processing_){
                auto delete_status = client_.DeleteObject(bucket_name_, object_name_template_);
                if(!delete_status.ok()){
                    std::cerr << "Failed to delete object after processing: " << delete_status.message() << std::endl;
                }else{
                    std::cout << "Object deleted after processing." << std::endl;
                }
            }

            return Message(file_content, object_name_template_);
        }catch(google::cloud::Status const& status){
            std::cerr << "google::cloud::Status thrown: " << status << "\n";
            throw std::runtime_error("Error pulling messages from gcp storage");
        }
    }
};


json gcp_bucket_connector_schema_ = {
    "gcp_bucket", {
        {"type", {
            {"type", "string"},
            {"enum", {"gcp_bucket"}},
            {"required", true}
        }},
        {"authbundle_id", {
            {"type", "string"},
            {"required", true}
        }},
        {"project_id", {
            {"type", "string"},
            {"required", true}
        }},
        {"bucket_name", {
            {"type", "string"},
            {"required", true}
        }},
        {"object_name", {
            {"type", "string"},
            {"required", true}
        }},
        {"delete_after_processing", {
            {"type", "boolean"},
            {"required", false}
        }}
    }
};


#endif  // __M2E_BRIDGE_GCP_BUCKET_CONNECTOR_H__