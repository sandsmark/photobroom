/*
 * Photo Broom - photos management tool.
 * Copyright (C) 2020  Michał Walenciak <Kicer86@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cnn_face_detector.hpp"

#include <dlib/dnn.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_transforms.h>
#include <dlib/matrix.h>

#include "helpers.hpp"


namespace dlib_api
{

    struct cnn_face_detection_model_v1::data
    {
        template <long num_filters, typename SUBNET> using con5d = dlib::con<num_filters,5,5,2,2,SUBNET>;
        template <long num_filters, typename SUBNET> using con5  = dlib::con<num_filters,5,5,1,1,SUBNET>;

        template <typename SUBNET> using downsampler  = dlib::relu<dlib::affine<con5d<32, dlib::relu<dlib::affine<con5d<32, dlib::relu<dlib::affine<con5d<16,SUBNET>>>>>>>>>;
        template <typename SUBNET> using rcon5  = dlib::relu<dlib::affine<con5<45,SUBNET>>>;

        using net_type = dlib::loss_mmod<dlib::con<1,9,9,1,1,rcon5<rcon5<rcon5<downsampler<dlib::input_rgb_image_pyramid<dlib::pyramid_down<6>>>>>>>>;

        net_type net;
    };


    cnn_face_detection_model_v1::cnn_face_detection_model_v1(const std::string& model_filename)
        : m_data(std::make_unique<data>())
    {
        dlib::deserialize(model_filename) >> m_data->net;
    }


    cnn_face_detection_model_v1::~cnn_face_detection_model_v1()
    {

    }


    std::vector<dlib::mmod_rect> cnn_face_detection_model_v1::detect(
        const QImage& qimage,
        const int upsample_num_times
    ) const
    {
        dlib::pyramid_down<2> pyr;
        std::vector<dlib::mmod_rect> face_rects;

        dlib::matrix<dlib::rgb_pixel> image = qimage_to_dlib_matrix(qimage);

        // Upsampling the image will allow us to detect smaller faces but will cause the
        // program to use more RAM and run longer.
        for (int i = 0; i < upsample_num_times; i++)
            pyramid_up(image, pyr);

        auto dets = m_data->net(image);

        // Scale the detection locations back to the original image size
        // if the image was upscaled.
        for (auto&& d : dets) {
            d.rect = pyr.rect_down(d.rect, upsample_num_times);
            face_rects.push_back(d);
        }

        return face_rects;
    }

}
