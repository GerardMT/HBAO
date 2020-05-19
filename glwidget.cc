// Author: Marc Comino 2020

#include <glwidget.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <limits>

#include "./mesh_io.h"
#include "./triangle_mesh.h"

namespace {

const double kFieldOfView = 60;
const double kZNear = 1;
const double kZFar = 10;

const char g_vert_file[] = "../shaders/g.vert";
const char g_frag_file[] = "../shaders/g.frag";

const char blur_vert_file[] = "../shaders/blur.vert";
const char blur_frag_file[] = "../shaders/blur.frag";

const char hbao_vert_file[] = "../shaders/hbao.vert";
const char hbao_frag_file[] = "../shaders/hbao.frag";

const char ao2_vert_file[] = "../shaders/ao2.vert";
const char ao2_frag_file[] = "../shaders/ao2.frag";

const char separable_ao_vert_file[] = "../shaders/separable_ao.vert";
const char separable_ao_frag_file[] = "../shaders/separable_ao.frag";

const int kVertexAttributeIdx = 0;
const int kNormalAttributeIdx = 1;

const float quad_vertices[] = {
  -1.0f,  1.0f, 0.0f,
  -1.0f, -1.0f, 0.0f,
   1.0f, -1.0f, 0.0f,
   1.0f, -1.0f, 0.0f,
   1.0f,  1.0f, 0.0f,
  -1.0f,  1.0f, 0.0f
};

bool ReadFile(const std::string filename, std::string *shader_source) {
  std::ifstream infile(filename.c_str());

  if (!infile.is_open() || !infile.good()) {
    std::cerr << "Error " + filename + " not found." << std::endl;
    return false;
  }

  std::stringstream stream;
  stream << infile.rdbuf();
  infile.close();

  *shader_source = stream.str();
  return true;
}

bool LoadProgram(const std::string &vertex, const std::string &fragment, QOpenGLShaderProgram &program) {
  std::string vertex_shader, fragment_shader;
  bool res = ReadFile(vertex, &vertex_shader) && ReadFile(fragment, &fragment_shader);

  if (res) {
    program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_shader.c_str());
    std::cout << program.log().toUtf8().constData();
    program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_shader.c_str());
    std::cout << program.log().toUtf8().constData();
    program.bindAttributeLocation("vertex", kVertexAttributeIdx);
    program.bindAttributeLocation("normal", kNormalAttributeIdx);
    program.link();
  }

  return res;
}

}  // namespace

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent), initialized_(false), width_(0.0), height_(0.0) {
  setFocusPolicy(Qt::StrongFocus);
}

GLWidget::~GLWidget() {
  delete g_program_;
  delete blur_program_;
  delete hbao_program_;
  delete ao2_program_;
  delete separable_ao_program_;

  if (initialized_) {
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &vno_);
    glDeleteBuffers(1, &ebo_);

    glDeleteVertexArrays(1, &quad_vao_);
    glDeleteBuffers(1, &quad_vbo_);

    glDeleteFramebuffers(1, &g_fbo_);
    glDeleteRenderbuffers(1, &g_rbo_);
    glDeleteTextures(1, &g_normal_depth_texture_);

    glDeleteFramebuffers(COLOR_FBOS, c_fbo_);
    glDeleteTextures(COLOR_FBOS, c_textures_);

    delete[] c_textures_;
  }
}

bool GLWidget::LoadModel(const QString &filename) {
  std::string file = filename.toUtf8().constData();
  size_t pos = file.find_last_of(".");
  std::string type = file.substr(pos + 1);

  std::unique_ptr<data_representation::TriangleMesh> mesh =
      std::make_unique<data_representation::TriangleMesh>();

  bool res = false;
  if (type.compare("ply") == 0) {
    res = data_representation::ReadFromPly(file, mesh.get());
  }

  if (res) {
    mesh_.reset(mesh.release());
    camera_.UpdateModel(mesh_->min_, mesh_->max_);

    // TODO(students): Create / Initialize buffers.
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, mesh_->vertices_.size() * sizeof(float), &mesh_->vertices_[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vno_);
    glBindBuffer(GL_ARRAY_BUFFER, vno_);
    glBufferData(GL_ARRAY_BUFFER, mesh_->normals_.size() * sizeof(float), &mesh_->normals_[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_->faces_.size() * sizeof(unsigned int), &mesh_->faces_[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
    // END.

    emit SetFaces(QString(std::to_string(mesh_->faces_.size() / 3).c_str()));
    emit SetVertices(
        QString(std::to_string(mesh_->vertices_.size() / 3).c_str()));
    return true;
  }

  return false;
}

void GLWidget::initializeGL() {
  glewInit();

  glEnable(GL_NORMALIZE);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);

  g_program_ = new QOpenGLShaderProgram();
  bool res = LoadProgram(g_vert_file, g_frag_file, *g_program_);
  blur_program_ = new QOpenGLShaderProgram();
  res &= LoadProgram(blur_vert_file, blur_frag_file, *blur_program_);
  hbao_program_ = new QOpenGLShaderProgram();
  res &= LoadProgram(hbao_vert_file, hbao_frag_file, *hbao_program_);
  ao2_program_ = new QOpenGLShaderProgram();
  res &= LoadProgram(ao2_vert_file, ao2_frag_file, *ao2_program_);
  separable_ao_program_ = new QOpenGLShaderProgram();
  res &= LoadProgram(separable_ao_vert_file, separable_ao_frag_file, *separable_ao_program_);

  if (!res) exit(0);

  // Quad
  glGenVertexArrays(1, &quad_vao_);
  glBindVertexArray(quad_vao_);

  glGenBuffers(1, &quad_vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);

  if (!LoadModel("../models/sphere.ply")) {
    return;
  }

  assert(COLOR_FBOS >= 0);
  unsigned int size = static_cast<unsigned int>(COLOR_FBOS);
  c_fbo_ = new GLuint[size];
  c_textures_ = new GLuint[size];

  ao_program_ = hbao_program_;

  initialized_ = true;
}

void GLWidget::resizeGL(int w, int h) {
  if (h == 0) h = 1;
  width_ = w;
  height_ = h;

  camera_.SetViewport(0, 0, w, h);
  camera_.SetProjection(kFieldOfView, kZNear, kZFar);

  if (resized_) {
    glDeleteFramebuffers(1, &g_fbo_);
    glDeleteRenderbuffers(1, &g_rbo_);
    glDeleteTextures(1, &g_normal_depth_texture_);

    glDeleteFramebuffers(COLOR_FBOS, c_fbo_);
    glDeleteTextures(COLOR_FBOS, c_textures_);
  }

  // G buffer
  glGenFramebuffers(1, &g_fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, g_fbo_);

  glGenTextures(1, &g_normal_depth_texture_);
  glBindTexture(GL_TEXTURE_2D, g_normal_depth_texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_normal_depth_texture_, 0);

  glGenRenderbuffers(1, &g_rbo_);
  glBindRenderbuffer(GL_RENDERBUFFER, g_rbo_);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, g_rbo_);

  // Color buffers, used for post processing
  glGenFramebuffers(COLOR_FBOS, c_fbo_);

  glGenTextures(COLOR_FBOS, c_textures_);
  for (GLsizei i = 0; i < COLOR_FBOS; ++i) {
    glBindFramebuffer(GL_FRAMEBUFFER, c_fbo_[i]);
    glBindTexture(GL_TEXTURE_2D, c_textures_[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, c_textures_[i], 0);
  }

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer is not complete!" << glGetError() << std::endl;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  resized_ = true;
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    camera_.StartRotating(event->x(), event->y());
  }
  if (event->button() == Qt::RightButton) {
    camera_.StartZooming(event->x(), event->y());
  }
  updateGL();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
  camera_.SetRotationX(event->y());
  camera_.SetRotationY(event->x());
  camera_.SafeZoom(event->y());
  updateGL();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    camera_.StopRotating(event->x(), event->y());
  }
  if (event->button() == Qt::RightButton) {
    camera_.StopZooming(event->x(), event->y());
  }
  updateGL();
}

void GLWidget::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Up) camera_.Zoom(-1);
  if (event->key() == Qt::Key_Down) camera_.Zoom(1);

  if (event->key() == Qt::Key_Left) camera_.Rotate(-1);
  if (event->key() == Qt::Key_Right) camera_.Rotate(1);

  if (event->key() == Qt::Key_W) camera_.Zoom(-1);
  if (event->key() == Qt::Key_S) camera_.Zoom(1);

  if (event->key() == Qt::Key_A) camera_.Rotate(-1);
  if (event->key() == Qt::Key_D) camera_.Rotate(1);

  if (event->key() == Qt::Key_R) {
    delete g_program_;
    g_program_ = new QOpenGLShaderProgram();
    LoadProgram(g_vert_file, g_frag_file, *g_program_);
    delete blur_program_;
    blur_program_ = new QOpenGLShaderProgram();
    LoadProgram(blur_vert_file, blur_frag_file, *blur_program_);
    delete hbao_program_;
    hbao_program_ = new QOpenGLShaderProgram();
    LoadProgram(hbao_vert_file, hbao_frag_file, *hbao_program_);
    delete ao2_program_;
    ao2_program_ = new QOpenGLShaderProgram();
    LoadProgram(ao2_vert_file, ao2_frag_file, *ao2_program_);
    delete separable_ao_program_;
    separable_ao_program_ = new QOpenGLShaderProgram();
    LoadProgram(separable_ao_vert_file, separable_ao_frag_file, *separable_ao_program_);
  }

  updateGL();
}

void GLWidget::paintGL() {
  if (initialized_) {
    camera_.SetViewport();

    Eigen::Matrix4f projection = camera_.SetProjection();
    Eigen::Matrix4f view = camera_.SetView();
    Eigen::Matrix4f model = camera_.SetModel();

    Eigen::Matrix4f t = view * model;
    Eigen::Matrix3f normal;
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j) normal(i, j) = t(i, j);

    normal = normal.inverse().transpose();

    if (mesh_ != nullptr) {
      // G Pass
      glBindFramebuffer(GL_FRAMEBUFFER, g_fbo_);

      glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glEnable(GL_DEPTH_TEST);

      g_program_->bind();
      glUniformMatrix4fv(g_program_->uniformLocation("projection"), 1, GL_FALSE, projection.data());
      glUniformMatrix4fv(g_program_->uniformLocation("view"), 1, GL_FALSE, view.data());
      glUniformMatrix4fv(g_program_->uniformLocation("model"), 1, GL_FALSE, model.data());
      glUniformMatrix3fv(g_program_->uniformLocation("normal_matrix"), 1, GL_FALSE, normal.data());

      glBindTexture(GL_TEXTURE_2D, g_normal_depth_texture_);

      // Draw model
      glBindVertexArray(vao_);
      assert(mesh_->faces_.size() <= std::numeric_limits<std::vector<int>::size_type>::max());
      glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh_->faces_.size()), GL_UNSIGNED_INT, nullptr);
      glBindVertexArray(0);

      bool h = true;
      if (blur_ > 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, c_fbo_[h]);
      } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
      }

      glClearColor(0.0f, 0.0f, 0.3f, 1.0f); // Not black!
      glClear(GL_COLOR_BUFFER_BIT);

      glDisable(GL_DEPTH_TEST);

      ao_program_->bind();
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, g_normal_depth_texture_);
      glUniform1i(ao_program_->uniformLocation("normalDepthTexture"), 0);

      glBindVertexArray(quad_vao_);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      glBindVertexArray(0);

      if (blur_ > 0) { // Blur. Render to ping pong color framebuffers
        for (unsigned int i = 0; i < blur_; ++i) {
          glBindFramebuffer(GL_FRAMEBUFFER, c_fbo_[!h]);
          blur_program_->bind();
          glUniform1i(blur_program_->uniformLocation("h"), h);
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, c_textures_[h]);

          glBindVertexArray(quad_vao_);
          glDrawArrays(GL_TRIANGLES, 0, 6);
          glBindVertexArray(0);

          h = !h;
        }

        // Blit color framebuffer to the default framebuffer. Could be done by rendering a quad.
        glBlitNamedFramebuffer(c_fbo_[h], 0, 0, 0, width_, height_, 0, 0, width_, height_, GL_COLOR_BUFFER_BIT, GL_NEAREST);
      }
    } else {
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
    }
  }
}

void GLWidget::set_hbao(bool v) {
  if (v) {
    ao_program_ = hbao_program_;
  }
  update();
}

void GLWidget::set_ao2(bool v) {
  if (v) {
    ao_program_ = ao2_program_;
  }
  update();
}

void GLWidget::set_separable_ao(bool v) {
  if (v) {
    ao_program_ = separable_ao_program_;
  }
  update();
}

void GLWidget::set_blur(int amount) {
  blur_ = amount;
  update();
}
