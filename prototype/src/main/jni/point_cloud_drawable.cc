/*
 * Copyright 2014 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sstream>

#include "tango-augmented-reality/point_cloud_drawable.h"

namespace {
    const std::string kPointCloudVertexShader =
            "precision mediump float;\n"
                    "precision mediump int;\n"
                    "attribute vec4 vertex;\n"
                    "uniform bool visible;\n"
                    "uniform mat4 mvp;\n"
                    "varying vec4 v_color;\n"
                    "void main() {\n"
                    "  gl_Position = mvp*vertex;\n"
                    "  v_color = vec4(0.0,0.0,0.0,0.0);\n"
//                    "if(visible){ v_color = vec4(vertex.z / 4.5,vertex.z / 4.5,vertex.z / 4.5,1.0);}\n" // grayscale
                    "if(visible){v_color = vertex;}\n" // colored
                    " gl_PointSize = 11.0;\n"
                    "}\n";
    const std::string kPointCloudFragmentShader =
            "precision mediump float;\n"
                    "precision mediump int;\n"
                    "varying vec4 v_color;\n"
                    "void main() {\n"
                    "  gl_FragColor = vec4(v_color);\n"
                    "}\n";

    const glm::mat4 kOpengGL_T_Depth =
            glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                      -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}  // namespace

namespace tango_augmented_reality {

    PointCloudDrawable::PointCloudDrawable() {
        shader_program_ = tango_gl::util::CreateProgram(
                kPointCloudVertexShader.c_str(), kPointCloudFragmentShader.c_str());

        mvp_handle_ = glGetUniformLocation(shader_program_, "mvp");
        vertices_visible_handle_ = glGetUniformLocation(shader_program_, "visible");

        vertices_handle_ = glGetAttribLocation(shader_program_, "vertex");
        glGenBuffers(1, &vertex_buffers_);
    }

    void PointCloudDrawable::DeleteGlResources() {
        if (vertex_buffers_) {
            glDeleteBuffers(1, &vertex_buffers_);
        }
        if (shader_program_) {
            glDeleteShader(shader_program_);
        }
    }

    void PointCloudDrawable::Render(glm::mat4 projection_mat, glm::mat4 view_mat,
                                    glm::mat4 model_mat,
                                    const std::vector <float> &vertices) {
        glUseProgram(shader_program_);
        vertices_visible_handle_ = glGetUniformLocation(shader_program_, "visible");
        if (visible) {
            glUniform1i(vertices_visible_handle_, GL_TRUE);
        } else {
            glUniform1i(vertices_visible_handle_, GL_FALSE);
        }

        mvp_handle_ = glGetUniformLocation(shader_program_, "mvp");
        // Calculate model view projection matrix.
        glm::mat4 mvp_mat = projection_mat * view_mat * model_mat * kOpengGL_T_Depth;
        glUniformMatrix4fv(mvp_handle_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(),
                     vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(vertices_handle_);
        glVertexAttribPointer(vertices_handle_, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_POINTS, 0, vertices.size() / 3);

        glUseProgram(0);
        tango_gl::util::CheckGlError("Pointcloud::Render()");
    }

    void PointCloudDrawable::SetVisibility(bool _visible) {
        visible = _visible;
    }

}  // namespace tango-augmented-reality
