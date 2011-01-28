/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

#include "ytsg-client.h"


static void ytsg_client_dispose (GObject *object);
static void ytsg_client_finalize (GObject *object);
static void ytsg_client_constructed (GObject *object);
static void ytsg_client_get_property (GObject    *object,
                                         guint       property_id,
                                         GValue     *value,
                                         GParamSpec *pspec);
static void ytsg_client_set_property (GObject      *object,
                                         guint         property_id,
                                         const GValue *value,
                                         GParamSpec   *pspec);

G_DEFINE_TYPE (YtsgClient, ytsg_client, G_TYPE_OBJECT);

#define YTSG_CLIENT_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_CLIENT, YtsgClientPrivate))

struct _YtsgClientPrivate
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
ytsg_client_class_init (YtsgClientClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsgClientPrivate));

  object_class->dispose      = ytsg_client_dispose;
  object_class->finalize     = ytsg_client_finalize;
  object_class->constructed  = ytsg_client_constructed;
  object_class->get_property = ytsg_client_get_property;
  object_class->set_property = ytsg_client_set_property;
}

static void
ytsg_client_constructed (GObject *object)
{
  YtsgClient *self = (YtsgClient*) object;

  if (G_OBJECT_CLASS (ytsg_client_parent_class)->constructed)
    G_OBJECT_CLASS (ytsg_client_parent_class)->constructed (object);
}

static void
ytsg_client_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  YtsgClient        *self = (YtsgClient*) object;
  YtsgClientPrivate *priv = self->priv;

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_client_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  YtsgClient        *self = (YtsgClient*) object;
  YtsgClientPrivate *priv = self->priv;

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_client_init (YtsgClient *self)
{
  self->priv = YTSG_CLIENT_GET_PRIVATE (self);
}

static void
ytsg_client_dispose (GObject *object)
{
  YtsgClient        *self = (YtsgClient*) object;
  YtsgClientPrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  G_OBJECT_CLASS (ytsg_client_parent_class)->dispose (object);
}

static void
ytsg_client_finalize (GObject *object)
{
  YtsgClient        *self = (YtsgClient*) object;
  YtsgClientPrivate *priv = self->priv;

  G_OBJECT_CLASS (ytsg_client_parent_class)->finalize (object);
}

