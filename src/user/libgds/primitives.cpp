#include <gds/libgds.h>
#include "libgds_internal.hpp"

#pragma GCC diagnostic ignored "-Wunused-parameter"

extern "C" void GDS_Dot(uint32_t x, uint32_t y, uint32_t colour, uint8_t size){
	gds_DrawingOp op;
	op.type = gds_DrawingOpType::Dot;
	op.Dot.x = x; op.Dot.y = y;
	op.Common.lineColour = colour;
	op.Common.lineWidth = size;
	GDS_AddDrawingOp(op);
}

extern "C" void GDS_Line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t colour, uint8_t width, uint32_t style){
	gds_DrawingOp op;
	op.type = gds_DrawingOpType::Line;
	op.Line.x1 = x1; op.Line.y1 = y1;
	op.Line.x2 = x2; op.Line.y2 = y2;
	op.Common.lineColour = colour;
	op.Common.lineWidth = width;
	op.Common.lineStyle = style;
	GDS_AddDrawingOp(op);
}

extern "C" void GDS_Box(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t lineColour, uint32_t fillColour, uint8_t lineWidth, uint32_t lineStyle, uint32_t fillStyle){
	gds_DrawingOp op;
	op.type = gds_DrawingOpType::Box;
	op.Box.x = x; op.Box.y = y;
	op.Box.w = w; op.Box.h = h;
	op.Common.lineColour = lineColour;
	op.Common.fillColour = fillColour;
	op.Common.lineWidth = lineWidth;
	op.Common.lineStyle = lineStyle;
	op.Common.fillStyle = fillStyle;
	GDS_AddDrawingOp(op);
}

extern "C" void GDS_Ellipse(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t lineColour, uint32_t fillColour, uint8_t lineWidth, uint32_t lineStyle, uint32_t fillStyle){
	gds_DrawingOp op;
	op.type = gds_DrawingOpType::Ellipse;
	op.Ellipse.x = x; op.Ellipse.y = y;
	op.Ellipse.w = w; op.Ellipse.h = h;
	op.Common.lineColour = lineColour;
	op.Common.fillColour = fillColour;
	op.Common.lineWidth = lineWidth;
	op.Common.lineStyle = lineStyle;
	op.Common.fillStyle = fillStyle;
	GDS_AddDrawingOp(op);
}

extern "C" void GDS_Arc(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t a1, uint32_t a2, uint32_t lineColour, uint32_t fillColour, uint8_t lineWidth, uint32_t lineStyle, uint32_t fillStyle){
	gds_DrawingOp op;
	op.type = gds_DrawingOpType::Arc;
	op.Arc.x = x; op.Arc.y = y;
	op.Arc.w = w; op.Arc.h = h;
	op.Arc.a1 = a1; op.Arc.a2 = a2;
	op.Common.lineColour = lineColour;
	op.Common.fillColour = fillColour;
	op.Common.lineWidth = lineWidth;
	op.Common.lineStyle = lineStyle;
	op.Common.fillStyle = fillStyle;
	GDS_AddDrawingOp(op);
}

extern "C" void GDS_Polygon(size_t points, gds_Point *pointData, bool closed,  uint32_t lineColour, uint32_t fillColour, uint8_t lineWidth, uint32_t lineStyle, uint32_t fillStyle){
	//Need to implement variable-length params...
}

extern "C" void GDS_Text(char *string, uint32_t fontID, uint32_t size, uint32_t colour, uint8_t style){
	//Need to implement variable-length params...
}
