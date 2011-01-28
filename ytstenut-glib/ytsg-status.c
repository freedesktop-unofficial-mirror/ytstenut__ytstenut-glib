/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

#include "ytsg-status.h"


static void ytsg_status_dispose (GObject *object);
static void ytsg_status_finalize (GObject *object);
static void ytsg_status_constructed (GObject *object);
static void ytsg_status_get_property (GObject    *object,
                                         guint       property_id,
                                         GValue     *value,
                                         GParamSpec *pspec);
static void ytsg_status_set_property (GObject      *object,
                                         guint         property_id,
                                         const GValue *value,
                                         GParamSpec   *pspec);

G_DEFINE_TYPE (YtsgStatus, ytsg_status, YTSG_TYPE_METADATA);

#define YTSG_STATUS_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_STATUS, YtsgStatusPrivate))

struct _YtsgStatusPrivate
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
ytsg_status_class_init (YtsgStatusClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsgStatusPrivate));

  object_class->dispose      = ytsg_status_dispose;
  object_class->finalize     = ytsg_status_finalize;
  object_class->constructed  = ytsg_status_constructed;
  object_class->get_property = ytsg_status_get_property;
  object_class->set_property = ytsg_status_set_property;
}

static void
ytsg_status_constructed (GObject *object)
{
  YtsgStatus *self = (YtsgStatus*) object;

  if (G_OBJECT_CLASS (ytsg_status_parent_class)->constructed)
    G_OBJECT_CLASS (ytsg_status_parent_class)->constructed (object);
}

static void
ytsg_status_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  YtsgStatus        *self = (YtsgStatus*) object;
  YtsgStatusPrivate *priv = self->priv;

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_status_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  YtsgStatus        *self = (YtsgStatus*) object;
  YtsgStatusPrivate *priv = self->priv;

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_status_init (YtsgStatus *self)
{
  self->priv = YTSG_STATUS_GET_PRIVATE (self);
}

static void
ytsg_status_dispose (GObject *object)
{
  YtsgStatus        *self = (YtsgStatus*) object;
  YtsgStatusPrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  G_OBJECT_CLASS (ytsg_status_parent_class)->dispose (object);
}

static void
ytsg_status_finalize (GObject *object)
{
  YtsgStatus        *self = (YtsgStatus*) object;
  YtsgStatusPrivate *priv = self->priv;

  G_OBJECT_CLASS (ytsg_status_parent_class)->finalize (object);
}

