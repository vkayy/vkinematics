#pragma once

#include <SFML/Graphics.hpp>

#include "../physics/solver.hpp"
#include "../renderer/renderer.hpp"
#include "../thread_pool/thread_pool.hpp"
#include "../utils/maths.hpp"

constexpr float ROPE_SEGMENT_LENGTH = 10.0f;
constexpr float DUMMY_RADIUS = 8.0f;

struct Simulation {
  Simulation(bool render_display, int32_t window_width, int32_t window_height,
             float min_radius, float max_radius, bool speed_colouring,
             int32_t max_object_count, int32_t framerate_limit,
             int32_t thread_count, int32_t substeps, int8_t collision_resolver,
             bool gravity_on, std::string name)
      : render_display{render_display}, window_height{window_height},
        window_width{window_width}, min_radius{min_radius},
        max_radius{max_radius}, collision_resolver{collision_resolver},
        thread_pool{tp::ThreadPool(thread_count)},
        settings{settings.antialiasingLevel = 4},
        window{sf::VideoMode(window_width, window_height), name,
               sf::Style::Default, settings},
        solver{sf::Vector2f(window_width, window_height),
               substeps,
               collision_resolver == 2 &&
                       window_width / 2 / max_radius / thread_count < 2
                   ? window_width / thread_count / 2
                   : max_radius * 2,
               max_object_count,
               framerate_limit,
               speed_colouring,
               thread_pool,
               gravity_on},
        renderer{window} {}

public:
  void spawnRigidBody(std::pair<float, float> spawn_position, int side_count,
                      float side_length) {
    solver.body_count++;
    const float radius = side_length / (2 * sin(M_PI / side_count));
    const int32_t side_points = side_length / DUMMY_RADIUS;
    const float segment_length = side_length / side_points;
    const sf::Vector2f centre{(1.0f - spawn_position.first) * window_width,
                              (1.0f - spawn_position.second) * window_height};
    const float angle_step = 2 * M_PI / side_count;

    std::vector<VerletObject *> vertices;

    for (int32_t i = 0; i < side_count; ++i) {
      float start_angle = i * angle_step;
      float end_angle = (i + 1) * angle_step;

      sf::Vector2f start_position =
          centre +
          sf::Vector2f(radius * cos(start_angle), radius * sin(start_angle));
      sf::Vector2f end_position =
          centre +
          sf::Vector2f(radius * cos(end_angle), radius * sin(end_angle));

      for (int32_t j = 0; j < side_points; ++j) {
        float t = static_cast<float>(j) / static_cast<float>(side_points - 1);
        sf::Vector2f position =
            start_position + t * (end_position - start_position);
        solver.body[solver.objects.size()] = solver.body_count - 1;
        VerletObject &object = solver.addObject(position, DUMMY_RADIUS);
        object.colour = getRainbowColour();
        vertices.push_back(&object);
      }
    }

    std::vector<VerletConstraint *> segments;
    const int32_t points = vertices.size();
    for (int32_t i = 0; i < points; i++) {
      VerletObject *current = vertices[i];
      VerletObject *next = vertices[(i + 1) % points];
      VerletConstraint &segment =
          solver.addConstraint(*current, *next, segment_length);
      segments.push_back(&segment);
      segment.in_body = true;
    }

    for (int32_t i = 0; i < points; i += side_points) {
      VerletObject *current = vertices[i];
      VerletObject *next = vertices[(i + side_points) % points];
      VerletConstraint &segment =
          solver.addConstraint(*current, *next, side_length);
      segments.push_back(&segment);
      segment.in_body = true;
    }

    // for (int32_t i=0; i<points; i++) {
    //     for (int32_t j=i+2; j<points; j++) {
    //         if (j != (i + points - 1) % points) {
    //             float diagonal_length = sqrt(pow(vertices[i]->curr_position.x
    //             - vertices[j]->curr_position.x, 2) +
    //                 pow(vertices[i]->curr_position.y -
    //                 vertices[j]->curr_position.y, 2));
    //             VerletConstraint &segment =
    //             solver.addConstraint(*vertices[i], *vertices[j],
    //             diagonal_length); segments.push_back(&segment);
    //             segment.in_body = true;
    //         }
    //     }
    // }

    solver.addRigidBody(vertices, segments, side_length);
    update();
    handleRender();
  }

  void spawnSquare(std::pair<float, float> spawn_position, float side_length) {
    solver.body_count++;
    const sf::Vector2f centre{(1.0f - spawn_position.first) * window_width,
                              (1.0f - spawn_position.second) * window_height};
    int32_t side_points = side_length / DUMMY_RADIUS;
    const int32_t segment_length = side_length / side_points;
    std::vector<VerletObject *> vertices;
    for (int32_t i = 0; i < side_points; i++) {
      solver.body[solver.objects.size()] = solver.body_count - 1;
      sf::Vector2f position =
          centre +
          sf::Vector2f(i * segment_length - side_length / 2, -side_length / 2);
      VerletObject &object = solver.addObject(position, DUMMY_RADIUS);
      object.colour = getRainbowColour();
      vertices.push_back(&object);
    }
    for (int32_t i = 0; i < side_points; i++) {
      solver.body[solver.objects.size()] = solver.body_count - 1;
      sf::Vector2f position =
          centre +
          sf::Vector2f(side_length / 2, i * segment_length - side_length / 2);
      VerletObject &object = solver.addObject(position, DUMMY_RADIUS);
      object.colour = getRainbowColour();
      vertices.push_back(&object);
    }
    for (int32_t i = 0; i < side_points; i++) {
      solver.body[solver.objects.size()] = solver.body_count - 1;
      sf::Vector2f position =
          centre +
          sf::Vector2f(side_length / 2 - i * segment_length, side_length / 2);
      VerletObject &object = solver.addObject(position, DUMMY_RADIUS);
      object.colour = getRainbowColour();
      vertices.push_back(&object);
    }
    for (int32_t i = 0; i < side_points; i++) {
      solver.body[solver.objects.size()] = solver.body_count - 1;
      sf::Vector2f position =
          centre +
          sf::Vector2f(-side_length / 2, side_length / 2 - i * segment_length);
      VerletObject &object = solver.addObject(position, DUMMY_RADIUS);
      object.colour = getRainbowColour();
      vertices.push_back(&object);
    }
    std::vector<VerletConstraint *> segments;
    const int32_t points = vertices.size();
    for (int32_t i = 0; i < points; i++) {
      VerletObject *current = vertices[i];
      VerletObject *next = vertices[(i + 1) % points];
      VerletConstraint &segment =
          solver.addConstraint(*current, *next, segment_length);
      segments.push_back(&segment);
      segment.in_body = true;
    }
    side_points = points / 4;
    for (int32_t i = 0; i < points; i += side_points) {
      VerletObject *current = vertices[i];
      VerletObject *diagonal1 = vertices[(i + side_points) % points];
      VerletObject *diagonal2 = vertices[(i + 2 * side_points) % points];
      VerletConstraint &segment1 =
          solver.addConstraint(*current, *diagonal1, side_length);
      VerletConstraint &segment2 =
          solver.addConstraint(*current, *diagonal2, sqrt(2) * side_length);
      segments.push_back(&segment1);
      segments.push_back(&segment2);
      segment1.in_body = true;
      segment2.in_body = true;
    }
    solver.addRigidBody(vertices, segments, side_length);
    update();
    handleRender();
  }

  VerletSoftBody &spawnSoftBody(std::pair<float, float> spawn_position,
                                float size_factor, float squish_factor) {
    solver.body_count++;
    const sf::Vector2f centre{(1.0f - spawn_position.first) * window_width,
                              (1.0f - spawn_position.second) * window_height};
    const int32_t radius = 8.0f * size_factor + 22.0f;
    const int32_t points = radius;
    const float angle_step = 360.0f / points;
    float circumference = 2 * M_PI * radius;
    float length = circumference * (1.0f + squish_factor * 0.3f) / points;

    std::vector<VerletObject *> vertices;
    vertices.reserve(points);
    for (int32_t i = 0; i < points; i++) {
      const float angle = angle_step * i;
      sf::Vector2f position = centre + sf::Vector2f(cos(angle), sin(angle));
      solver.body[solver.objects.size()] = solver.body_count - 1;
      VerletObject &object = solver.addObject(position, DUMMY_RADIUS);
      object.colour = getRainbowColour();
      vertices.push_back(&object);
    }

    std::vector<VerletConstraint *> segments;
    for (int32_t i = 0; i < points; i++) {
      VerletObject *current = vertices[i];
      VerletObject *next = vertices[(i + 1) % points];
      VerletConstraint &segment = solver.addConstraint(*current, *next, length);
      segments.push_back(&segment);
      segment.in_body = true;
    }
    VerletSoftBody &soft_body = solver.addSoftBody(vertices, segments, radius);
    update();
    handleRender();
    return soft_body;
  }

  void spawnRope(int32_t length, std::pair<float, float> spawn_position,
                 float spawn_delay, float radius) {
    int32_t total = 0;
    solver.body_count++;
    const sf::Vector2f fixed_position{
        (1.0f - spawn_position.first) * window_width,
        (1.0f - spawn_position.second) * window_height};
    VerletObject *last_object = nullptr;
    while (window.isOpen() && total <= length) {
      handleWindowEvents();
      if (clock.getElapsedTime().asSeconds() >= spawn_delay) {
        clock.restart();
        spawnRopeObject(total, last_object, radius, fixed_position,
                        total == length);
        total++;
      }
      update();
      handleRender();
    }
  }

  void spawnFree(int32_t count, std::pair<float, float> spawn_position,
                 float spawn_speed, float spawn_delay, float spawn_angle) {
    int total = 0;
    const sf::Vector2f spawn_position_vector{
        (1.0f - spawn_position.first) * window_width,
        (1.0f - spawn_position.second) * window_height};
    const sf::Vector2f spawn_angle_vector{cos(spawn_angle), sin(spawn_angle)};
    while (window.isOpen() && total < count) {
      handleWindowEvents();
      if (clock.getElapsedTime().asSeconds() >= spawn_delay) {
        clock.restart();
        spawnFreeObject(spawn_position_vector, spawn_angle_vector,
                        rng.getRange(min_radius, max_radius), spawn_speed);
        total++;
      }
      update();
      handleRender();
    }
  }

  void idle() {
    while (window.isOpen()) {
      handleWindowEvents();
      update();
      handleRender();
    }
  }

private:
  bool render_display;
  int32_t window_width;
  int32_t window_height;
  float min_radius;
  float max_radius;
  int8_t collision_resolver;
  sf::ContextSettings settings;
  sf::RenderWindow window;
  tp::ThreadPool thread_pool;
  Solver solver;
  Renderer renderer;
  sf::Clock clock;
  RNG<float> rng;

  sf::Color getRainbowColour() {
    const float time = solver.time;
    const float r = sin(time);
    const float g = sin(time + 0.33f * 2.0f * M_PI);
    const float b = sin(time + 0.66f * 2.0f * M_PI);
    return {static_cast<uint8_t>(255.0f * r * r),
            static_cast<uint8_t>(255.0f * g * g),
            static_cast<uint8_t>(255.0f * b * b)};
  }

  void spawnRopeObject(int32_t total, VerletObject *&last_object, float radius,
                       sf::Vector2f fixed_position, bool is_final) {
    solver.body[solver.objects.size()] = solver.body_count - 1;
    const sf::Vector2f spawn_position =
        total ? last_object->curr_position +
                    sf::Vector2f(0.0f, ROPE_SEGMENT_LENGTH +
                                           (is_final ? radius : DUMMY_RADIUS))
              : fixed_position;
    VerletObject &object = solver.addObject(
        spawn_position, (is_final ? radius : DUMMY_RADIUS), !last_object);
    object.colour = getRainbowColour();
    if (last_object) {
      VerletConstraint &constraint =
          solver.addConstraint(*last_object, object, ROPE_SEGMENT_LENGTH);
      constraint.in_body = true;
    }
    last_object = &object;
  }

  void spawnFreeObject(sf::Vector2f position, sf::Vector2f angle, float radius,
                       float speed) {
    VerletObject &object = solver.addObject(position, radius);
    object.colour = getRainbowColour();
    solver.setObjectVelocity(object, speed * angle);
  }

  void handleWindowEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed ||
          sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
        window.close();
      } else {
        solver.setAttractor(sf::Keyboard::isKeyPressed(sf::Keyboard::A));
        solver.setRepeller(sf::Keyboard::isKeyPressed(sf::Keyboard::R));
        solver.setSpeedUp(sf::Keyboard::isKeyPressed(sf::Keyboard::S));
        solver.setSlowDown(sf::Keyboard::isKeyPressed(sf::Keyboard::W));
        solver.setSlomo(sf::Keyboard::isKeyPressed(sf::Keyboard::F));
      }
    }
  }

  void update() {
    switch (collision_resolver) {
    case 0:
      solver.updateThreaded();
      break;
    case 1:
      solver.updateCellular();
      break;
    case 2:
      solver.updateNaive();
      break;
    default:
      solver.updateThreaded();
    }
  }

  void handleRender() {
    window.clear(sf::Color::White);
    renderer.render(solver);
    window.display();
  }
};
