#include "videosourceraspberrycam.hh"

VideoSourceRaspberryCam::VideoSourceRaspberryCam(QString sourceIdentifier, QObject *parent) : VideoSource(sourceIdentifier, parent), m_camera(nullptr)
{
}

VideoSourceRaspberryCam::~VideoSourceRaspberryCam() {
  shutdownCamera();
}

void VideoSourceRaspberryCam::nextImage() {

  if (m_camera == nullptr) {
    bool ok = initializeCamera();
    if (!ok) {
      m_requestExit = true;
      return;
    }
  }

  // m_camera->GrabImage...
  // convert to m_currentImage
  //return;

}

bool VideoSourceRaspberryCam::initializeCamera()
{
  // initialize camera here!
  // return false on error
  return false;
}

bool VideoSourceRaspberryCam::shutdownCamera()
{
  // stop and delete camera here
  return false;
}
