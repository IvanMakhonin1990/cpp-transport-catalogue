#define _USE_MATH_DEFINES

#include <cmath>
#include <iomanip>

#include "svg.h"

using namespace std::literals;

namespace shapes {
    using namespace svg;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(svg::Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "  <circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point)
    {
        m_points.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "  <polyline points=\""sv;
        for (size_t i = 0; i < m_points.size(); ++i) {
            out << m_points[i].x << ","sv << m_points[i].y << ((m_points.size() - 1) == i ? ""sv : " "sv);
        }
        
        out << "\"";
        RenderAttrs(context.out); 
        out <<"/>";
    }

    Text& Text::SetPosition(Point pos)
    {
        m_position = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset)
    {
        m_offset = std::move(offset);
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size)
    {
        m_size = std::move(size);
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family)
    {
        m_font_family = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight)
    {
        m_font_weight = std::move(font_weight);
        return *this;
    }

    void replaceAll(std::string& str, const std::string& from, const std::string& to) {
        if (from.empty())
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    Text& Text::SetData(std::string data)
    {
        m_data = std::move(data);
        replaceAll(m_data, "&", "&amp;");
        replaceAll(m_data, "\"", "&quot;");
        replaceAll(m_data, "'", "&apos;");
        replaceAll(m_data, "<", "&lt;");
        replaceAll(m_data, ">", "&gt;");

        return *this;
    }
    void Text::RenderObject(const RenderContext& context) const
    {
        //<text x="20" y="35" class="small">My</text>
        auto& out = context.out;
        out << "  <text";
        RenderAttrs(context.out);
        out << " x=\""sv << m_position.x << "\" y=\"" << m_position.y << "\" dx=\"" << m_offset.x << "\"";
        out << " dy=\"" << m_offset.y << "\" font-size=\"" << m_size << "\"";
        if (!m_font_family.empty()) {
            out << " font-family=\"" << m_font_family << "\"";
        }
        if (!m_font_weight.empty()) {
            out << " font-weight=\"" << m_font_weight << "\"";
        }
        out << ">" << m_data << "</text>";
    }

    RenderContext::RenderContext() :out(std::cout) { /*out << std::setprecision(6);*/ }
}

namespace svg {

using namespace shapes;

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
  m_objects.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
  RenderContext context(out);
  context.out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
  context.out << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)"
              << std::endl;
  for (auto& obj : m_objects) {
    obj->Render(context);
  }
  context.out << "</svg>";
}

Document::Document() {}

Triangle::Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
    : p1_(p1), p2_(p2), p3_(p3) {}

void Triangle::Draw(svg::ObjectContainer& container) const {
  container.Add(
      svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
}

Polyline Star::CreateStar() const {
  Polyline polyline;
  for (int i = 0; i <= m_num_rays; ++i) {
    double angle = 2 * M_PI * (i % m_num_rays) / m_num_rays;
    polyline.AddPoint({m_center.x + m_outer_rad * sin(angle),
                       m_center.y - m_outer_rad * cos(angle)});
    if (i == m_num_rays) {
      break;
    }
    angle += M_PI / m_num_rays;
    polyline.AddPoint({m_center.x + m_inner_rad * sin(angle),
                       m_center.y - m_inner_rad * cos(angle)});
  }
  return polyline;
}

Star::Star(Point center, double outer_rad, double inner_rad, int num_rays)
    : m_center(center),
      m_outer_rad(outer_rad),
      m_inner_rad(inner_rad),
      m_num_rays(num_rays) {}

void Star::Draw(svg::ObjectContainer& container) const {
  container.Add(CreateStar().SetFillColor("red").SetStrokeColor("black"));
}

Snowman::Snowman(svg::Point head_center, double radius)
    : m_head_center(head_center), m_radius(radius) {}

void Snowman::Draw(svg::ObjectContainer& container) const {
  container.Add(
      Circle()
          .SetCenter({m_head_center.x, m_head_center.y + m_radius * 5.0})
          .SetRadius(m_radius * 2.0)
          .SetFillColor("rgb(240,240,240)")
          .SetStrokeColor("black"));
  container.Add(
      Circle()
          .SetCenter({m_head_center.x, m_head_center.y + m_radius * 2.0})
          .SetRadius(m_radius * 1.5)
          .SetFillColor("rgb(240,240,240)")
          .SetStrokeColor("black"));
  container.Add(Circle()
                    .SetCenter(m_head_center)
                    .SetRadius(m_radius)
                    .SetFillColor("rgb(240,240,240)")
                    .SetStrokeColor("black"));
}

std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& linejoin) {
  switch (linejoin) {
    case StrokeLineJoin::ARCS:
      os << "arcs";
      break;
    case StrokeLineJoin::BEVEL:
      os << "bevel";
      break;
    case StrokeLineJoin::MITER:
      os << "miter";
      break;
    case StrokeLineJoin::MITER_CLIP:
      os << "miter-clip";
      break;
    case StrokeLineJoin::ROUND:
      os << "round";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Color& color) {
  os << std::visit(ColorPrinter{}, color);
  return os;
}

std::ostream& operator<<(std::ostream& os, const StrokeLineCap& linecap) {
  switch (linecap) {
    case StrokeLineCap::BUTT:
      os << "butt";
      break;
    case StrokeLineCap::ROUND:
      os << "round";
      break;
    case StrokeLineCap::SQUARE:
      os << "square";
      break;
  }
  return os;
}
std::ostream& operator<<(std::ostream& os, const Rgb& color) {
  os << color.ToString();
  return os;
}

std::ostream& operator<<(std::ostream& os, const Rgba& color) {
  os << color.ToString();
  return os;
}

Rgb::Rgb() : red(0), green(0), blue(0) {}

Rgb::Rgb(uint8_t red_, uint8_t green_, uint8_t blue_)
    : red(red_), green(green_), blue(blue_) {}

std::string Rgb::ToString() const {
  std::stringstream ss;
  ss << "rgb(" << static_cast<uint16_t>(red) << "," << static_cast<uint16_t>(green) << "," << static_cast<uint16_t>(blue) << ")";
  return ss.str();
}

Rgba::Rgba() : red(0), green(0), blue(0), opacity(1.0) {}

Rgba::Rgba(uint8_t red_, uint8_t green_, uint8_t blue_, double opacity_)
    : red(red_), green(green_), blue(blue_), opacity(opacity_) {}

std::string Rgba::ToString() const {
  std::stringstream ss;
  ss  << "rgba(" << static_cast<uint16_t>(red) << "," << static_cast<uint16_t>(green) << "," << static_cast<uint16_t>(blue) << ","
     << opacity << ")";
  return ss.str();
}

std::string ColorPrinter::operator()(std::monostate) const {
  return NoneColor; }

std::string ColorPrinter::operator()(const std::string& string_color) const {
  if (string_color.empty()) {
    return NoneColor;
  }
  return string_color;
}

std::string ColorPrinter::operator()(const Rgb& rgb_color) const {
  return rgb_color.ToString();
}
std::string ColorPrinter::operator()(const Rgba& rgba_color) const {
  return rgba_color.ToString();
}
}  // namespace svg