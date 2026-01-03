/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2026 Pluraf Embedded AB <code@pluraf.com>

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


#ifndef __M2E_BRIDGE_IMAGE_FT_H__
#define __M2E_BRIDGE_IMAGE_FT_H__


#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>

#include "filtra.h"


struct ImageFTOperation
{
    enum class Type{ UNKN, RESIZE };

    static std::string to_string(ImageFTOperation::Type v)
    {
        switch(v){
            case Type::UNKN: return "unkn";
            case Type::RESIZE: return "resize";
            default: throw std::invalid_argument("Invalid ImageFTOperation");
        }
    }

    static ImageFTOperation::Type from_string(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::tolower(c); });

        if( str == "unkn" ){ return ImageFTOperation::Type::UNKN; }
        if( str == "resize" ){ return ImageFTOperation::Type::RESIZE; }

        throw std::invalid_argument("Unknown format: " + str);
    }
};


class ImageFT: public Filtra
{
    ImageFTOperation::Type operation_;
    string extra_;
public:
    ImageFT(PipelineIface const & pi, json const & config): Filtra(pi, config)
    {
        operation_ = ImageFTOperation::from_string(config.at("operation").get<string>());
        extra_ = config.value("extra", "");
    }

    string process_message(MessageWrapper &msg_w) override
    {
        // Load the image
        auto payload = msg_w.msg().get_payload_uchar();
        auto original = cv::imdecode(payload, cv::IMREAD_COLOR);

        cv::Size new_size(800, 600);  // width x height
        cv::Mat resized;
        cv::resize(original, resized, new_size, 0, 0, cv::INTER_LINEAR);

        vector<int> params = { cv::IMWRITE_JPEG_QUALITY, 90 };

        if( extra_.empty() )
        {
            cv::imencode(".jpeg", resized, payload, params);
        }
        else
        {
            uchars payload;
            cv::imencode(".jpeg", resized, payload, params);
            msg_w.get_extra().add_extra(extra_, std::move(payload));
        }

        msg_w.pass();
        return "";
    }

    static pair<string, json> get_schema()
    {
        json schema = Filtra::get_schema();
        return {"image", schema};
    }
};


#endif  // __M2E_BRIDGE_IMAGE_FT_H__