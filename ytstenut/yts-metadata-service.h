/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

#ifndef _YTS_METADATA_SERVICE_H
#define _YTS_METADATA_SERVICE_H

#include <ytstenut/yts-error.h>
#include <ytstenut/yts-message.h>
#include <ytstenut/yts-service.h>
#include <ytstenut/yts-status.h>

G_BEGIN_DECLS

#define YTS_TYPE_METADATA_SERVICE                                      \
   (yts_metadata_service_get_type())

#define YTS_METADATA_SERVICE(obj)                                      \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                YTS_TYPE_METADATA_SERVICE,             \
                                YtsMetadataService))
#define YTS_METADATA_SERVICE_CLASS(klass)                              \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             YTS_TYPE_METADATA_SERVICE,                \
                             YtsMetadataServiceClass))
#define YTS_IS_METADATA_SERVICE(obj)                                   \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                YTS_TYPE_METADATA_SERVICE))
#define YTS_IS_METADATA_SERVICE_CLASS(klass)                           \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             YTS_TYPE_METADATA_SERVICE))
#define YTS_METADATA_SERVICE_GET_CLASS(obj)                            \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               YTS_TYPE_METADATA_SERVICE,              \
                               YtsMetadataServiceClass))

typedef struct _YtsMetadataService        YtsMetadataService;
typedef struct _YtsMetadataServiceClass   YtsMetadataServiceClass;
typedef struct _YtsMetadataServicePrivate YtsMetadataServicePrivate;

/**
 * YtsMetadataServiceClass:
 *
 * #YtsMetadataService class.
 */
struct _YtsMetadataServiceClass
{
  /*< private >*/
  YtsServiceClass parent_class;

  /*< public >*/
  void (*received_status)  (YtsMetadataService *service, YtsStatus  *status);
  void (*received_message) (YtsMetadataService *service, YtsMessage *message);
};

/**
 * YtsMetadataService:
 *
 * Ytstenut service.
 */
struct _YtsMetadataService
{
  /*< private >*/
  YtsService parent;

  /*< private >*/
  YtsMetadataServicePrivate *priv;
};

GType yts_metadata_service_get_type (void) G_GNUC_CONST;

YtsError yts_metadata_service_send_metadata (YtsMetadataService *service,
                                               YtsMetadata        *metadata);

G_END_DECLS

#endif /* _YTS_METADATA_SERVICE_H */
