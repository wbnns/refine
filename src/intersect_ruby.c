
#include "ruby.h"
#include "intersect.h"

static VALUE intersect_side( VALUE self,
                             VALUE tri0, VALUE tri1, VALUE tri2, VALUE node )
{
  int i;
  double t0[3], t1[3], t2[3], n[3];
  for (i=0;i<3;i++){
    t0[i] = NUM2DBL(rb_ary_entry(tri0, i));
    t1[i] = NUM2DBL(rb_ary_entry(tri1, i));
    t2[i] = NUM2DBL(rb_ary_entry(tri2, i));
    n[i]  = NUM2DBL(rb_ary_entry(node, i));
  }
  return INT2FIX( intersectSide( t0, t1, t2, n ) );
}

static VALUE intersect_triangleNode( VALUE self, 
				     VALUE tri0, VALUE tri1, VALUE tri2, 
				     VALUE node )
{
  int i;
  double t0[3], t1[3], t2[3], n[3];
  for (i=0;i<3;i++){
    t0[i] = NUM2DBL(rb_ary_entry(tri0, i));
    t1[i] = NUM2DBL(rb_ary_entry(tri1, i));
    t2[i] = NUM2DBL(rb_ary_entry(tri2, i));
    n[i]  = NUM2DBL(rb_ary_entry(node, i));
  }
  return (intersectTriangleNode(t0,t1,t2,n)?Qtrue:Qfalse);
}

static VALUE intersect_triangleSegment( VALUE self, 
				       VALUE tri0, VALUE tri1, VALUE tri2, 
				       VALUE node0, VALUE node1 )
{
  int i;
  double t0[3], t1[3], t2[3], n0[3], n1[3];
  for (i=0;i<3;i++){
    t0[i] = NUM2DBL(rb_ary_entry(tri0, i));
    t1[i] = NUM2DBL(rb_ary_entry(tri1, i));
    t2[i] = NUM2DBL(rb_ary_entry(tri2, i));
    n0[i] = NUM2DBL(rb_ary_entry(node0, i));
    n1[i] = NUM2DBL(rb_ary_entry(node1, i));
  }
  return (intersectTriangleSegment(t0,t1,t2,n0,n1)?Qtrue:Qfalse);
}

static VALUE intersect_tetSegment( VALUE self, 
				   VALUE vert0, VALUE vert1, 
				   VALUE vert2, VALUE vert3, 
				   VALUE node0, VALUE node1 )
{
  int i;
  double v0[3], v1[3], v2[3], v3[3], n0[3], n1[3];
  for (i=0;i<3;i++){
    v0[i] = NUM2DBL(rb_ary_entry(vert0, i));
    v1[i] = NUM2DBL(rb_ary_entry(vert1, i));
    v2[i] = NUM2DBL(rb_ary_entry(vert2, i));
    v3[i] = NUM2DBL(rb_ary_entry(vert3, i));
    n0[i] = NUM2DBL(rb_ary_entry(node0, i));
    n1[i] = NUM2DBL(rb_ary_entry(node1, i));
  }
  return (intersectTetSegment(v0,v1,v2,v3,n0,n1)?Qtrue:Qfalse);
}

VALUE cIntersect;

void Init_Intersect(  )
{
  cIntersect = rb_define_class( "Intersect", rb_cObject );
  rb_define_method( cIntersect, "side", intersect_side, 4 );
  rb_define_method( cIntersect, "triangleNode", intersect_triangleNode, 4 );
  rb_define_method( cIntersect, "triangleSegment", intersect_triangleSegment, 5 );
  rb_define_method( cIntersect, "tetSegment", intersect_tetSegment, 6 );
}
