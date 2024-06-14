#include <acfutils/glew.h>
#include <acfutils/mt_cairo_render.h>


namespace cairo_utils
{
    inline void prepare_cairo_context(cairo_t* cr, vect3_t color, int line_width)
    {
        cairo_set_source_rgb(cr, color.x, color.y, color.z);
        if(line_width > 0)
        {
            cairo_set_line_width(cr, line_width);
        }
        else
        {
            cairo_set_line_width(cr, 1);
        }
    }

    inline void draw_rect(cairo_t* cr, vect2_t pos, vect2_t sz, vect3_t color, int line_width=-1)
    {
        prepare_cairo_context(cr, color, line_width);

        cairo_rectangle(cr, pos.x, pos.y, pos.x + sz.x, pos.y + sz.y);
        cairo_stroke_preserve(cr);

        if(line_width < 0)
        {
            cairo_fill(cr);
        }
    }

    inline void draw_tri(cairo_t* cr, vect2_t v1, vect2_t v2, vect2_t v3, vect3_t color, int line_width=-1)
    {
        prepare_cairo_context(cr, color, line_width);

        cairo_move_to(cr, v1.x, v1.y);
        cairo_line_to(cr, v2.x, v2.y);
        cairo_line_to(cr, v3.x, v3.y);
        cairo_close_path(cr);

        cairo_stroke_preserve(cr);

        if(line_width < 0)
        {
            cairo_fill(cr);
        }
    }
}
