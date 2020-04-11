// Author: Marc Comino 2020

#include <glwidget.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "./mesh_io.h"
#include "./triangle_mesh.h"

namespace {

const double kFieldOfView = 60;
const double kZNear = 0.0001;
const double kZFar = 10;

const char gVertexShaderFile[] = "../shaders/g.vert";
const char gFragmentShaderFile[] = "../shaders/g.frag";

const char lightVertexShaderFile[] = "../shaders/light.vert";
const char lightFragmentShaderFile[] = "../shaders/light.frag";


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

bool LoadProgram(const std::string &vertex, const std::string &fragment, QOpenGLShaderProgram *program) {
  std::string vertex_shader, fragment_shader;
  bool res = ReadFile(vertex, &vertex_shader) && ReadFile(fragment, &fragment_shader);

  if (res) {
    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_shader.c_str());
    std::cout << program->log().toUtf8().constData();
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_shader.c_str());
    std::cout << program->log().toUtf8().constData();
    program->bindAttributeLocation("vertex", kVertexAttributeIdx);
    program->bindAttributeLocation("normal", kNormalAttributeIdx);
    program->link();
  }

  return res;
}

}  // namespace

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent), initialized_(false), width_(0.0), height_(0.0) {
  setFocusPolicy(Qt::StrongFocus);
}

GLWidget::~GLWidget() {
  if (initialized_) {
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &vno_);
    glDeleteBuffers(1, &ebo_);

    glDeleteVertexArrays(1, &quad_vao_);
    glDeleteBuffers(1, &quad_vbo_);

    glDeleteFramebuffers(1, &fbo_);
    glDeleteRenderbuffers(1, &rbo_);

    glDeleteTextures(1, &normalDepthTexture_);
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
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(2);

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

  g_program_ = std::make_unique<QOpenGLShaderProgram>();
  bool res = LoadProgram(gVertexShaderFile, gFragmentShaderFile, g_program_.get());

  light_program_ = std::make_unique<QOpenGLShaderProgram>();
  res &= LoadProgram(lightVertexShaderFile, lightFragmentShaderFile, light_program_.get());

  if (!res) exit(0);

  glGenFramebuffers(1, &fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width_, height_);

  glGenTextures(1, &normalDepthTexture_);
  glBindTexture(GL_TEXTURE_2D, normalDepthTexture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, static_cast<int>(width_), static_cast<int>(height_), 0, GL_RGB, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);

  glGenRenderbuffers(1, &rbo_);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width_, height_);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Quad
  glGenVertexArrays(1, &quad_vao_);
  glBindVertexArray(quad_vao_);

  glGenBuffers(1, &quad_vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);

  LoadModel("../models/sphere.ply");

  initialized_ = true;
}

void GLWidget::resizeGL(int w, int h) {
  if (h == 0) h = 1;
  width_ = w;
  height_ = h;

  camera_.SetViewport(0, 0, w, h);
  camera_.SetProjection(kFieldOfView, kZNear, kZFar);
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
    g_program_.reset();
    g_program_ = std::make_unique<QOpenGLShaderProgram>();
    LoadProgram(gVertexShaderFile, gFragmentShaderFile, g_program_.get());

    light_program_.reset();
    light_program_ = std::make_unique<QOpenGLShaderProgram>();
    LoadProgram(lightVertexShaderFile, lightFragmentShaderFile, light_program_.get());
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
      glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
      glBindRenderbuffer(GL_RENDERBUFFER, rbo_);

      glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      g_program_->bind();
      glUniformMatrix4fv( g_program_->uniformLocation("projection"), 1, GL_FALSE, projection.data());
      glUniformMatrix4fv(g_program_->uniformLocation("view"), 1, GL_FALSE, view.data());
      glUniformMatrix4fv(g_program_->uniformLocation("model"), 1, GL_FALSE, model.data());
      glUniformMatrix3fv(g_program_->uniformLocation("normal_matrix"), 1, GL_FALSE, normal.data());

      glBindTexture(GL_TEXTURE_2D, normalDepthTexture_);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normalDepthTexture_, 0);

      // Draw model
      glBindVertexArray(vao_);
      glDrawElements(GL_TRIANGLES, mesh_->faces_.size(), GL_UNSIGNED_INT, nullptr);
      glBindVertexArray(0);

      // Light pass
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      glClearColor(0.0f, 0.0f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      glDisable(GL_DEPTH_TEST);

      light_program_->bind();

      glBindTexture(GL_TEXTURE_2D, normalDepthTexture_);

      glBindVertexArray(quad_vao_);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      glBindVertexArray(0);
    } else {
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
    }
  }
}
