// Author: Marc Comino 2020

#ifndef CAMERA_H_
#define CAMERA_H_

#include <eigen3/Eigen/Geometry>

namespace data_visualization {

const double kMaxCameraDistance = 3.0;
const double kMinCameraDistance = -kMaxCameraDistance;

const double kMinRotationX = -M_PI / 2;
const double MaxRotationX = M_PI / 2;

const double AngleIncrement = 0.01;

class Camera {
 private:
  /**
   * @brief distance_ Zoom distance.
   */
  double distance_;

  /**
   * @brief step_ A parameter that determines the influence of the mouse
   * movement over the camera.
   */
  double step_;

  /**
   * @brief rotating_ Indicates whether the camera is rotating.
   */
  bool rotating_;

  /**
   * @brief zooming_ Indicates whether the camera is zooming.
   */
  bool zooming_;

  /**
   * @brief zooming_ Indicates whether the camera is panning.
   */
  bool panning_;

  /**
   * @brief rotation_y_ Camera rotation around the Y axis.
   */
  double rotation_y_;

  /**
   * @brief rotation_y_ Camera rotation around the X axis.
   */
  double rotation_x_;

  /**
   * @brief pan_x_ Camera pan in the horizontal Viewport direction.
   */
  double pan_x_;

  /**
   * @brief pan_y_ Camera pan in the vertical Viewport direction.
   */
  double pan_y_;

  /**
   * @brief current_x_ Should store the mouse current X to compute
   * displacements.
   */
  double current_x_;

  /**
   * @brief current_y_ Should store the mouse current Y to compute
   * displacements.
   */
  double current_y_;

  /**
   * @brief viewport_x_ Viewport X position.
   */
  int viewport_x_;

  /**
   * @brief viewport_y_ Viewport Y position.
   */
  int viewport_y_;

  /**
   * @brief viewport_width_ Viewport width;
   */
  int viewport_width_;

  /**
   * @brief viewport_height_ Viewport height;
   */
  int viewport_height_;

  /**
   * @brief centering_x_ Translation applied to the modeling transform. Tries to
   * center a model bounding box.
   */
  double centering_x_;

  /**
   * @brief centering_y_ Translation applied to the modeling transform. Tries to
   * center a model bounding box.
   */
  double centering_y_;

  /**
   * @brief centering_z_ Translation applied to the modeling transform. Tries to
   * center a model bounding box.
   */
  double centering_z_;

  /**
   * @brief scaling_ Scaling applied to the modeling transform. Tries to make
   * unit length the longest edge of a model bounding box.
   */
  double scaling_;

  /**
   * @brief field_of_view_ Field of view for a perspective camera.
   */
  double field_of_view_;

  /**
   * @brief z_near_ Near plane Z for a perspective camera.
   */
  double z_near_;

  /**
   * @brief z_far_ Far plane Z for a perspective camera.
   */
  double z_far_;

 public:
  /**
   * @brief Camera Constructor of the class.
   */
  Camera();

  /**
   * @brief Camera Destructor of the class.
   */
  ~Camera() {}

  /**
   * @brief SetViewport Calls glViewport with the current viewport position,
   * width and height and stores these values.
   * @param x Viewport position x.
   * @param y Viewport position y.
   * @param w Viewport width-
   * @param h Viewport height.
   */
  void SetViewport(int x, int y, int w, int h);

  /**
   * @brief SetViewport Calls glViewport with the current viewport position,
   * width and height.
   */
  void SetViewport() const;

  /**
   * @brief SetIdentity Returns an identity matrix.
   * @return An identity matrix.
   */
  Eigen::Matrix4f SetIdentity() const;

  /**
   * @brief SetModel Returns a model transform matrix that centers the last
   * "updated" model and scales its bounding box longest edge to unit length.
   * @return A modeling transform.
   */
  Eigen::Matrix4f SetModel() const;

  /**
   * @brief SetView Computed the viewing matrix given the rotation_x_ around the
   * x axis, rotation_y_ around the y axis, a pan with translation (pan_x_,
   * pan_y_) and a zoom out of distance_.
   * @return A viewing transform.
   */
  Eigen::Matrix4f SetView() const;

  /**
   * @brief SetProjection Computes the projection matrix for a perspective
   * camera, and stores its parameters.
   * @param fov Field of view for a perspective camera.
   * @param znear Near plane Z for a perspective camera.
   * @param zfar Far plane Z for a perspective camera.
   * @return A perspective camera matrix transform.
   */
  Eigen::Matrix4f SetProjection(double fov, double znear, double zfar);

  /**
   * @brief SetProjection Computes the projection matrix for a perspective
   * camera, using the stored parameters.
   * @return A perspective camera matrix transform.
   */
  Eigen::Matrix4f SetProjection() const;

  /**
   * @brief Zoom Zooms the camera in the direction given by the modifier.
   * @param modifier Sign of the zooming direction.
   */
  void Zoom(double modifier);

  /**
   * @brief Zoom If zooming is active, zooms the camera in the direction given
   * by the modifier.
   * @param modifier Sign of the zooming direction.
   */
  void SafeZoom(double modifier);

  /**
   * @brief SafePan If panning is active, pans the camera by the displaced mouse
   * position.
   * @param x New mouse X position.
   * @param y New mouse Y position.
   */
  void SafePan(double x, double y);

  /**
   * @brief Rotate Rotates the camera around the Y axis.
   * @param modifier Sign of the rotation direction.
   */
  void Rotate(double modifier);

  /**
   * @brief UpdateModel Updates the intrinsic parameters to compute a modeling
   * transform that centers the bounding box of the model and makes its longest
   * edge unit length.
   * @param min Minimum point of the model bounding box.
   * @param max Maximum point of the model bounding box.
   */
  void UpdateModel(Eigen::Vector3f min, Eigen::Vector3f max);

  /**
   * @brief SetRotationX If rotating is active, rotates the camera around the X
   * axis.
   * @param y New mouse Y position.
   */
  void SetRotationX(double y);

  /**
   * @brief SetRotationY If rotating is active, Rotates the camera around the Y
   * axis.
   * @param x New mouse X position.
   */
  void SetRotationY(double x);

  /**
   * @brief StartRotating Allows camera rotation with starting mouse position
   * (x,y).
   * @param x New mouse X position.
   * @param y New mouse Y position.
   */
  void StartRotating(double x, double y);

  /**
   * @brief StopRotating Finishes camera rotation with ending mouse position
   * (x,y).
   * @param x New mouse X position.
   * @param y New mouse Y position.
   */
  void StopRotating(double x, double y);

  /**
   * @brief StartZooming Allows camera zooming with starting mouse position
   * (x,y).
   * @param x New mouse X position.
   * @param y New mouse Y position.
   */
  void StartZooming(double x, double y);

  /**
   * @brief StopZooming Finishes camera zooming with ending mouse position
   * (x,y).
   * @param x New mouse X position.
   * @param y New mouse Y position.
   */
  void StopZooming(double x, double y);

  /**
   * @brief StartPanning Allows camera panning with starting mouse position
   * (x,y).
   * @param x New mouse X position.
   * @param y New mouse Y position.
   */
  void StartPanning(double x, double y);

  /**
   * @brief StopPanning Finishes camera panning with ending mouse position
   * (x,y).
   * @param x New mouse X position.
   * @param y New mouse Y position.
   */
  void StopPanning(double x, double y);

  /**
   * @brief SetCameraStep Modifies the camera step.
   * @param step New camera step.
   */
  void SetCameraStep(double step);
};

}  //  namespace data_visualization

#endif  //  CAMERA_H_
