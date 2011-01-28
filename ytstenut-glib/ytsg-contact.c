/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

#include "ytsg-contact.h"


static void ytsg_contact_dispose (GObject *object);
static void ytsg_contact_finalize (GObject *object);
static void ytsg_contact_constructed (GObject *object);
static void ytsg_contact_get_property (GObject    *object,
                                         guint       property_id,
                                         GValue     *value,
                                         GParamSpec *pspec);
static void ytsg_contact_set_property (GObject      *object,
                                         guint         property_id,
                                         const GValue *value,
                                         GParamSpec   *pspec);

G_DEFINE_TYPE (YtsgContact, ytsg_contact, G_TYPE_OBJECT);

#define YTSG_CONTACT_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_CONTACT, YtsgContactPrivate))

struct _YtsgContactPrivate
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
ytsg_contact_class_init (YtsgContactClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsgContactPrivate));

  object_class->dispose      = ytsg_contact_dispose;
  object_class->finalize     = ytsg_contact_finalize;
  object_class->constructed  = ytsg_contact_constructed;
  object_class->get_property = ytsg_contact_get_property;
  object_class->set_property = ytsg_contact_set_property;
}

static void
ytsg_contact_constructed (GObject *object)
{
  YtsgContact *self = (YtsgContact*) object;

  if (G_OBJECT_CLASS (ytsg_contact_parent_class)->constructed)
    G_OBJECT_CLASS (ytsg_contact_parent_class)->constructed (object);
}

static void
ytsg_contact_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  YtsgContact        *self = (YtsgContact*) object;
  YtsgContactPrivate *priv = self->priv;

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_contact_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  YtsgContact        *self = (YtsgContact*) object;
  YtsgContactPrivate *priv = self->priv;

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_contact_init (YtsgContact *self)
{
  self->priv = YTSG_CONTACT_GET_PRIVATE (self);
}

static void
ytsg_contact_dispose (GObject *object)
{
  YtsgContact        *self = (YtsgContact*) object;
  YtsgContactPrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  G_OBJECT_CLASS (ytsg_contact_parent_class)->dispose (object);
}

static void
ytsg_contact_finalize (GObject *object)
{
  YtsgContact        *self = (YtsgContact*) object;
  YtsgContactPrivate *priv = self->priv;

  G_OBJECT_CLASS (ytsg_contact_parent_class)->finalize (object);
}

