/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

#include "ytsg-metadata.h"


static void ytsg_metadata_dispose (GObject *object);
static void ytsg_metadata_finalize (GObject *object);
static void ytsg_metadata_constructed (GObject *object);
static void ytsg_metadata_get_property (GObject    *object,
                                         guint       property_id,
                                         GValue     *value,
                                         GParamSpec *pspec);
static void ytsg_metadata_set_property (GObject      *object,
                                         guint         property_id,
                                         const GValue *value,
                                         GParamSpec   *pspec);

G_DEFINE_TYPE (YtsgMetadata, ytsg_metadata, G_TYPE_OBJECT);

#define YTSG_METADATA_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_METADATA, YtsgMetadataPrivate))

struct _YtsgMetadataPrivate
{
  guint disposed : 1;
};

enum
{
  N_SIGNALS,
};

enum
{
  PROP_0,
};

static guint signals[N_SIGNALS] = {0};

static void
ytsg_metadata_class_init (YtsgMetadataClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsgMetadataPrivate));

  object_class->dispose      = ytsg_metadata_dispose;
  object_class->finalize     = ytsg_metadata_finalize;
  object_class->constructed  = ytsg_metadata_constructed;
  object_class->get_property = ytsg_metadata_get_property;
  object_class->set_property = ytsg_metadata_set_property;
}

static void
ytsg_metadata_constructed (GObject *object)
{
  YtsgMetadata *self = (YtsgMetadata*) object;

  if (G_OBJECT_CLASS (ytsg_metadata_parent_class)->constructed)
    G_OBJECT_CLASS (ytsg_metadata_parent_class)->constructed (object);
}

static void
ytsg_metadata_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  YtsgMetadata        *self = (YtsgMetadata*) object;
  YtsgMetadataPrivate *priv = self->priv;

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_metadata_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  YtsgMetadata        *self = (YtsgMetadata*) object;
  YtsgMetadataPrivate *priv = self->priv;

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_metadata_init (YtsgMetadata *self)
{
  self->priv = YTSG_METADATA_GET_PRIVATE (self);
}

static void
ytsg_metadata_dispose (GObject *object)
{
  YtsgMetadata        *self = (YtsgMetadata*) object;
  YtsgMetadataPrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  G_OBJECT_CLASS (ytsg_metadata_parent_class)->dispose (object);
}

static void
ytsg_metadata_finalize (GObject *object)
{
  YtsgMetadata        *self = (YtsgMetadata*) object;
  YtsgMetadataPrivate *priv = self->priv;

  G_OBJECT_CLASS (ytsg_metadata_parent_class)->finalize (object);
}

