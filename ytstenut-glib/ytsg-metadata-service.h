/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

#ifndef _YTSG_METADATA_SERVICE_H
#define _YTSG_METADATA_SERVICE_H

#include <ytstenut-glib/ytsg-error.h>
#include <ytstenut-glib/ytsg-message.h>
#include <ytstenut-glib/ytsg-service.h>
#include <ytstenut-glib/ytsg-status.h>

G_BEGIN_DECLS

#define YTSG_TYPE_METADATA_SERVICE                                      \
   (ytsg_metadata_service_get_type())

#define YTSG_METADATA_SERVICE(obj)                                      \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                YTSG_TYPE_METADATA_SERVICE,             \
                                YtsgMetadataService))
#define YTSG_METADATA_SERVICE_CLASS(klass)                              \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             YTSG_TYPE_METADATA_SERVICE,                \
                             YtsgMetadataServiceClass))
#define YTSG_IS_METADATA_SERVICE(obj)                                   \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                YTSG_TYPE_METADATA_SERVICE))
#define YTSG_IS_METADATA_SERVICE_CLASS(klass)                           \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             YTSG_TYPE_METADATA_SERVICE))
#define YTSG_METADATA_SERVICE_GET_CLASS(obj)                            \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               YTSG_TYPE_METADATA_SERVICE,              \
                               YtsgMetadataServiceClass))

typedef struct _YtsgMetadataService        YtsgMetadataService;
typedef struct _YtsgMetadataServiceClass   YtsgMetadataServiceClass;
typedef struct _YtsgMetadataServicePrivate YtsgMetadataServicePrivate;

/**
 * YtsgMetadataServiceClass:
 *
 * #YtsgMetadataService class.
 */
struct _YtsgMetadataServiceClass
{
  /*< private >*/
  YtsgServiceClass parent_class;

  /*< public >*/
  void (*received_status)  (YtsgMetadataService *service, YtsgStatus  *status);
  void (*received_message) (YtsgMetadataService *service, YtsgMessage *message);
};

/**
 * YtsgMetadataService:
 *
 * Ytstenut service.
 */
struct _YtsgMetadataService
{
  /*< private >*/
  YtsgService parent;

  /*< private >*/
  YtsgMetadataServicePrivate *priv;
};

GType ytsg_metadata_service_get_type (void) G_GNUC_CONST;

YtsgError ytsg_metadata_service_send_metadata (YtsgMetadataService *service,
                                               YtsgMetadata        *metadata);

G_END_DECLS

#endif /* _YTSG_METADATA_SERVICE_H */
