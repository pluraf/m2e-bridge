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


#ifndef __M2E_BRIDGE_RESIZE_MODIFIER_H__
#define __M2E_BRIDGE_RESIZE_MODIFIER_H__


#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>

#include "m2e_aliases.h"
#include "modifier_internal.h"


class ResizeModifier: public ModifierInternal
{
    int width_;

public:
    ResizeModifier( string const & params )
        :ModifierInternal(params)
    {
        width_ = std::stoi(params);
    }

    uchars modify( std::span<std::byte const> data ) override
    {
        // Load the image
        cv::_InputArray input { reinterpret_cast<uchar const *>( data.data() ), static_cast<int>(data.size()) };
        auto original = cv::imdecode( input, cv::IMREAD_COLOR);

        double ratio = static_cast<double>(width_) / original.cols;
        int height = static_cast<int>(original.rows * ratio);
        cv::Size new_size(width_, height);  // width x height
        cv::Mat resized;
        cv::resize(original, resized, new_size, 0, 0, cv::INTER_LINEAR);

        vector<int> params = { cv::IMWRITE_JPEG_QUALITY, 90 };

        uchars payload;
        cv::imencode(".jpeg", resized, payload, params);

        return payload;
    }

    static json get_schema()
    {
        return json {};
    }
};


#endif  // __M2E_BRIDGE_RESIZE_MODIFIER_H__
