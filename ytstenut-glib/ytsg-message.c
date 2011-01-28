/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

#include "ytsg-message.h"


static void ytsg_message_dispose (GObject *object);
static void ytsg_message_finalize (GObject *object);
static void ytsg_message_constructed (GObject *object);
static void ytsg_message_get_property (GObject    *object,
                                         guint       property_id,
                                         GValue     *value,
                                         GParamSpec *pspec);
static void ytsg_message_set_property (GObject      *object,
                                         guint         property_id,
                                         const GValue *value,
                                         GParamSpec   *pspec);

G_DEFINE_TYPE (YtsgMessage, ytsg_message, YTSG_TYPE_METADATA);

#define YTSG_MESSAGE_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_MESSAGE, YtsgMessagePrivate))

struct _YtsgMessagePrivate
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
ytsg_message_class_init (YtsgMessageClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsgMessagePrivate));

  object_class->dispose      = ytsg_message_dispose;
  object_class->finalize     = ytsg_message_finalize;
  object_class->constructed  = ytsg_message_constructed;
  object_class->get_property = ytsg_message_get_property;
  object_class->set_property = ytsg_message_set_property;
}

static void
ytsg_message_constructed (GObject *object)
{
  YtsgMessage *self = (YtsgMessage*) object;

  if (G_OBJECT_CLASS (ytsg_message_parent_class)->constructed)
    G_OBJECT_CLASS (ytsg_message_parent_class)->constructed (object);
}

static void
ytsg_message_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  YtsgMessage        *self = (YtsgMessage*) object;
  YtsgMessagePrivate *priv = self->priv;

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_message_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  YtsgMessage        *self = (YtsgMessage*) object;
  YtsgMessagePrivate *priv = self->priv;

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_message_init (YtsgMessage *self)
{
  self->priv = YTSG_MESSAGE_GET_PRIVATE (self);
}

static void
ytsg_message_dispose (GObject *object)
{
  YtsgMessage        *self = (YtsgMessage*) object;
  YtsgMessagePrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  G_OBJECT_CLASS (ytsg_message_parent_class)->dispose (object);
}

static void
ytsg_message_finalize (GObject *object)
{
  YtsgMessage        *self = (YtsgMessage*) object;
  YtsgMessagePrivate *priv = self->priv;

  G_OBJECT_CLASS (ytsg_message_parent_class)->finalize (object);
}

