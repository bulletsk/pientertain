#ifndef VIDEOSOURCE_HH
#define VIDEOSOURCE_HH

#include <QThread>
#include <QImage>
#include <QMutex>
#include <QJsonObject>

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

  explicit VideoSource(const QString &sourceIdentifier="", QObject *parent = nullptr);
  virtual ~VideoSource();

  static VideoSource *createVideoSource(const QString &identifier, VideoSourceType type);

  QVector<QPoint> corners() const;

  int area() const;
  int smooth() const;

  void stop();

  void readSettings();
  void writeSettings();

signals:
  void newColors( QVector<QColor> colorVector );
  void statusChanged(QString status, bool err);
  void latestImage( const QImage &image );
  void cameraSettingsChanged( QJsonObject jsonString );

public slots:
  void onRequestImage();
  void setCorners( const QVector<QPoint> &corners);
  virtual void setCameraSettings (const QJsonObject &json );

protected:
  void run() override;

  virtual void nextImage() = 0;
  void calculateColors();


protected:
  QVector<QColor> m_colors;
  QVector<QPoint> m_corners;
  QList<QPoint> m_measurePoints;
  QImage m_currentImage;
  QImage m_latestImage;
  QString m_identifier;
  bool m_requestExit;
  int m_areaSize;
  int m_smoothCount;

  QJsonObject m_settings;


  QMutex m_imageLock;

};

#endif // VIDEOSOURCE_HH
