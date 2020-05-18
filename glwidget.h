// Author: Marc Comino 2020

#ifndef GLWIDGET_H_
#define GLWIDGET_H_

#include <GL/glew.h>
#include <QGLWidget>
#include <QImage>
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QString>

#include <memory>

#include "./camera.h"
#include "./triangle_mesh.h"

class GLWidget : public QGLWidget {
  Q_OBJECT

 public:
  explicit GLWidget(QWidget *parent = nullptr);
  ~GLWidget();

  /**
   * @brief LoadModel Loads a PLY model at the filename path into the mesh_ data
   * structure.
   * @param filename Path to the PLY model.
   * @return Whether it was able to load the model.
   */
  bool LoadModel(const QString &filename);

 protected:
  /**
   * @brief initializeGL Initializes OpenGL variables and loads, compiles and
   * links shaders.
   */
  void initializeGL();

  /**
   * @brief resizeGL Resizes the viewport.
   * @param w New viewport width.
   * @param h New viewport height.
   */
  void resizeGL(int w, int h);

  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void keyPressEvent(QKeyEvent *event);

 private:
  /**
   * @brief program_ The G buffer shader program.
   */
  QOpenGLShaderProgram *g_program_ = nullptr;

  /**
   * @brief program_ The blur shader program.
   */
  QOpenGLShaderProgram *blur_program_ = nullptr;

  /**
   * @brief program_ The HBAO shader program.
   */
  QOpenGLShaderProgram *hbao_program_ = nullptr;

  /**
   * @brief program_ The other AO shader program.
   */
  QOpenGLShaderProgram *ao2_program_ = nullptr; // TODO Plz, rename me

  /**
   * @brief program_ The Separable SSAO shader program.
   */
  QOpenGLShaderProgram *separable_ao_program_ = nullptr;

  /**
   * @brief camera_ Class that computes the multiple camera transform matrices.
   */
  data_visualization::Camera camera_;

  /**
   * @brief mesh_ Data structure representing a triangle mesh.
   */
  std::unique_ptr<data_representation::TriangleMesh> mesh_;

  /**
   * @brief initialized_ Whether the widget has finished initializations.
   */
  bool initialized_ = false;

  /**
   * @brief width_ Viewport current width.
   */
  float width_;

  /**
   * @brief height_ Viewport current height.
   */
  float height_;

  QOpenGLShaderProgram *ao_program_;

  GLuint g_fbo_;
  GLuint g_rbo_;
  GLuint g_normal_depth_texture_;

  const GLsizei COLOR_FBOS = 2;

  GLuint *c_fbo_;
  GLuint *c_textures_;

  GLuint vao_;
  GLuint vbo_;
  GLuint vno_;
  GLuint ebo_;

  GLuint quad_vao_;
  GLuint quad_vbo_;

  bool resized_ = false;

  int blur_ = 0;

 protected slots:
  /**
   * @brief paintGL Function that handles rendering the scene.
   */
  void paintGL();

  void set_hbao(bool v);

  void set_ao2(bool v);

  void set_separable_ao(bool v);

  void set_blur(int amount);

 signals:
  /**
   * @brief SetFaces Signal that updates the interface label "Faces".
   */
  void SetFaces(QString);

  /**
   * @brief SetFaces Signal that updates the interface label "Vertices".
   */
  void SetVertices(QString);

  /**
   * @brief SetFaces Signal that updates the interface label "Framerate".
   */
  void SetFramerate(QString);
};

#endif  //  GLWIDGET_H_
