#ifndef VIDEOSOURCE_HH
#define VIDEOSOURCE_HH

#include <QThread>
#include <QImage>
#include <QMutex>

class VideoSource : public QThread
{
  Q_OBJECT
public:

  enum VideoSourceType {
    Image=0,
    Video,
    Camera
  };

  enum Corner {
    TopLeft=0,
    TopCenter,
    TopRight,
    CenterLeft,
    CenterCenter,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight,
    CornerLast
  };

  enum CropCorner {
    CCTopLeft=0,
    CCTopRight,
    CCBottomLeft,
    CCBottomRight
  };

  explicit VideoSource(QString sourceIdentifier="", QObject *parent = nullptr);
  virtual ~VideoSource();

  static VideoSource *createVideoSource(QString identifier, VideoSourceType type);

  QVector<QPoint> corners() const;
  void setCorners( const QVector<QPoint> &corners);

  virtual QSize imageSize() const;
  QImage currentImage() const;

  void stop();

  void readSettings();
  void writeSettings();

signals:
  void newColors( QVector<QColor> colorVector );
  void statusChanged(QString status, bool err);
  void latestImage( const QImage &image );

public slots:
  void onRequestImage();

protected:
  void run() override;

  virtual void nextImage() = 0;
  void calculateColors();


protected:
  QVector<QColor> m_colors;
  QVector<QPoint> m_corners;
  QImage m_currentImage;
  QImage m_latestImage;
  QString m_identifier;

  bool m_requestExit;

  QMutex m_imageLock;

};

#endif // VIDEOSOURCE_HH
