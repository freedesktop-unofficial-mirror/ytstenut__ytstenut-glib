/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

#include "ytsg-roster.h"


static void ytsg_roster_dispose (GObject *object);
static void ytsg_roster_finalize (GObject *object);
static void ytsg_roster_constructed (GObject *object);
static void ytsg_roster_get_property (GObject    *object,
                                         guint       property_id,
                                         GValue     *value,
                                         GParamSpec *pspec);
static void ytsg_roster_set_property (GObject      *object,
                                         guint         property_id,
                                         const GValue *value,
                                         GParamSpec   *pspec);

G_DEFINE_TYPE (YtsgRoster, ytsg_roster, G_TYPE_OBJECT);

#define YTSG_ROSTER_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_ROSTER, YtsgRosterPrivate))

struct _YtsgRosterPrivate
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
ytsg_roster_class_init (YtsgRosterClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsgRosterPrivate));

  object_class->dispose      = ytsg_roster_dispose;
  object_class->finalize     = ytsg_roster_finalize;
  object_class->constructed  = ytsg_roster_constructed;
  object_class->get_property = ytsg_roster_get_property;
  object_class->set_property = ytsg_roster_set_property;
}

static void
ytsg_roster_constructed (GObject *object)
{
  YtsgRoster *self = (YtsgRoster*) object;

  if (G_OBJECT_CLASS (ytsg_roster_parent_class)->constructed)
    G_OBJECT_CLASS (ytsg_roster_parent_class)->constructed (object);
}

static void
ytsg_roster_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  YtsgRoster        *self = (YtsgRoster*) object;
  YtsgRosterPrivate *priv = self->priv;

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_roster_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  YtsgRoster        *self = (YtsgRoster*) object;
  YtsgRosterPrivate *priv = self->priv;

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_roster_init (YtsgRoster *self)
{
  self->priv = YTSG_ROSTER_GET_PRIVATE (self);
}

static void
ytsg_roster_dispose (GObject *object)
{
  YtsgRoster        *self = (YtsgRoster*) object;
  YtsgRosterPrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  G_OBJECT_CLASS (ytsg_roster_parent_class)->dispose (object);
}

static void
ytsg_roster_finalize (GObject *object)
{
  YtsgRoster        *self = (YtsgRoster*) object;
  YtsgRosterPrivate *priv = self->priv;

  G_OBJECT_CLASS (ytsg_roster_parent_class)->finalize (object);
}

