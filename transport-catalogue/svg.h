#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <iomanip>
#include <optional>
#include <variant>
#include <sstream>

namespace svg {
struct Point {
  Point() = default;
  Point(double x, double y) : x(x), y(y) {}
  double x = 0;
  double y = 0;
};

inline const std::string NoneColor{"none"};

struct Rgba {
  Rgba();
  Rgba(uint8_t red_, uint8_t green_, uint8_t blue_, double opacity_);
  std::string ToString() const;

  uint8_t red=0;
  uint8_t green=0;
  uint8_t blue=0;
  double opacity=1.0;
};

struct Rgb {
  Rgb();
  Rgb(uint8_t red_, uint8_t green_, uint8_t blue_);
  std::string ToString() const;

  uint8_t red=0;
  uint8_t green=0;
  uint8_t blue=0;
};

std::ostream& operator<<(std::ostream& os, const Rgb& color);

struct ColorPrinter {
  std::string operator()(std::monostate) const;

  std::string operator()(const std::string& string_color) const;

  std::string operator()(const Rgb& rgb_color) const;

  std::string operator()(const Rgba& rgba_color) const;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

std::ostream& operator<<(std::ostream& os, const Color& color);

enum class StrokeLineCap {
  BUTT,
  ROUND,
  SQUARE,
};
std::ostream& operator<<(std::ostream& os, const StrokeLineCap& linecap);

enum class StrokeLineJoin {
  ARCS,
  BEVEL,
  MITER,
  MITER_CLIP,
  ROUND,
};

std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& linejoin);

template <typename Owner>
class PathProps {
 public:
  Owner& SetFillColor(Color color) {
    fill_color_ = std::move(color);
    return AsOwner();
  }
  Owner& SetStrokeColor(Color color) {
    stroke_color_ = std::move(color);
    return AsOwner();
  }

  Owner& SetStrokeWidth(double width) {
    stroke_width_ = std::move(width);
    return AsOwner();
  }

  Owner& SetStrokeLineCap(StrokeLineCap linecap) {
    stroke_linecap_ = std::move(linecap);
    return AsOwner();
  }

  Owner& SetStrokeLineJoin(StrokeLineJoin linejoin) {
    stroke_linejoin_ = std::move(linejoin);
    return AsOwner();
  }

 protected:
  ~PathProps() = default;

  void RenderAttrs(std::ostream& out) const {
    using namespace std::literals;
    
    if (fill_color_) {
      out << " fill=\""sv << std::visit(ColorPrinter{}, *fill_color_) << "\""sv;
    }
    if (stroke_color_) {
      out << " stroke=\""sv << std::visit(ColorPrinter{}, *stroke_color_) << "\""sv;
    }
    if (stroke_width_) {
      out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
    }
    if (stroke_linecap_) {
      out << " stroke-linecap=\""sv << *stroke_linecap_ << "\""sv;
    }
    if (stroke_linejoin_) {
      out << " stroke-linejoin=\""sv << *stroke_linejoin_ << "\""sv;
    }
  }

 private:
  Owner& AsOwner() {
    // static_cast безопасно преобразует *this к Owner&,
    // если класс Owner — наследник PathProps
    return static_cast<Owner&>(*this);
  }

  std::optional<Color> fill_color_;
  std::optional<Color> stroke_color_;
  std::optional<double> stroke_width_;
  std::optional<StrokeLineCap> stroke_linecap_;
  std::optional<StrokeLineJoin> stroke_linejoin_;
};
}  // namespace svg

namespace shapes {

    using namespace svg;
    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext();

        RenderContext(std::ostream& out)
            : out(out) {
            //out << std::setprecision(6);
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
            //out << std::setprecision(6);
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };


    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    protected:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(svg::Point center);
        Circle& SetRadius(double radius);

    protected:
        void RenderObject(const RenderContext& context) const final;

    private:
        Point center_;
        double radius_ = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(svg::Point point);

        /*
         * Прочие методы и данные, необходимые для реализации элемента <polyline>
         */
    private:
        void RenderObject(const RenderContext& context) const override;
        std::vector<svg::Point> m_points;

    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(svg::Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(svg::Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;

    private:
        svg::Point m_position = { 0,0 };
        svg::Point m_offset = { 0,0 };
        uint32_t m_size = 1;
        std::string m_font_family;
        std::string m_font_weight;
        std::string m_data;
        // Прочие данные и методы, необходимые для реализации элемента <text>
    };

}



namespace svg {

    using namespace shapes;
    
    class ObjectContainer {
    public:
        virtual void AddPtr(std::unique_ptr<shapes::Object>&& obj) = 0;
        template<typename T> void Add(const T& obj) {
            this->AddPtr(std::make_unique<T>(std::move(obj)));
        }

        virtual ~ObjectContainer() {}
    };

    class Document : public ObjectContainer {
    public:
        Document();
        /*
         Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
         Пример использования:
         Document doc;
         doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
        */
        // void Add(???);

        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<shapes::Object>&& obj) override;


        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;


    private:
        std::vector<std::unique_ptr<Object>> m_objects;
        RenderContext m_context;
        // Прочие методы и данные, необходимые для реализации класса Document
    };

    // Интерфейс Drawable задаёт объекты, которые можно нарисовать с помощью Graphics
    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() {};
    };

    class Triangle : public svg::Drawable {
    public:
        Triangle(svg::Point p1, svg::Point p2, svg::Point p3);

        // Реализует метод Draw интерфейса svg::Drawable
        void Draw(svg::ObjectContainer& container) const override;

    private:
        svg::Point p1_, p2_, p3_;
    };

    class Star : public Drawable {
    public:
        Star(Point center, double outer_rad, double inner_rad, int num_rays);

        // Реализует метод Draw интерфейса svg::Drawable
        void Draw(svg::ObjectContainer& container) const override;

    private:
        svg::Point p1_, p2_, p3_;
        shapes::Polyline CreateStar() const;
        Point m_center;
        double m_outer_rad;
        double m_inner_rad;
        int m_num_rays;
    };

    class Snowman : public Drawable {
    public:
        Snowman(svg::Point head_center, double radius);

        void Draw(svg::ObjectContainer& container) const override;

    private:
        svg::Point m_head_center;
        double m_radius;
    };

}  // namespace svg

