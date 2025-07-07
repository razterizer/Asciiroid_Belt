//
//  asciiroids.cpp
//  Asciiroids
//
//  Created by Rasmus Anthin on 2025-07-05.
//

#include "Keyboard.h"
#include "TitleScreen.h"
#include "InstructionsScreen.h"

#include <Termin8or/GameEngine.h>
#include <Termin8or/SpriteHandler.h>
#include <Termin8or/ASCII_Fonts.h>
#include <Termin8or/Dynamics/RigidBody.h>
#include <Termin8or/Dynamics/DynamicsSystem.h>
#include <Termin8or/Dynamics/CollisionHandler.h>
#include <8Beat/AudioSourceHandler.h>
#include <8Beat/ChipTuneEngine.h>

///////////////////////////////

class Game : public GameEngine<40, 100>
{
public:
  Game(int argc, char** argv, const GameEngineParams& params)
    : GameEngine(argv[0], params)
  {
  //#ifndef _WIN32
    GameEngine::set_real_fps(15);
    GameEngine::set_sim_delay_us(50'000);
    GameEngine::set_anim_rate(0, 5); // Spaceship
    GameEngine::set_anim_rate(1, 3); // Asteroids
    GameEngine::set_anim_rate(2, 5); // UFO AI
  //#endif
    if (argc >= 2)
      GameEngine::set_real_fps(static_cast<float>(atoi(argv[1])));
  }
  
  ~Game()
  {
    audio.remove_source(src_fx_shot);
    audio.remove_source(src_fx_explosion);
    audio.remove_source(src_fx_ufo_shot);
    audio.remove_source(src_fx_ufo_propulsion);
  }

  virtual void generate_data() override
  {
    try
    {
      std::string tune_path = get_exe_folder();
#ifndef _WIN32
      const char* xcode_env = std::getenv("RUNNING_FROM_XCODE");
      if (xcode_env != nullptr)
        tune_path = "../../../../../../../../Documents/xcode/Pilot_Episode/Pilot_Episode/"; // #FIXME: Find a better solution!
#endif
    
      if (chip_tune.load_tune(folder::join_path({ tune_path, "music.ct" })))
      {
          //chip_tune.play_tune();
          chip_tune.play_tune_async();
          chip_tune.wait_for_completion();
      }
    }
    catch (const std::exception& e)
    {
      std::cerr << "Caught exception: " << e.what() << std::endl;
    }
    
    src_fx_shot = audio.create_stream_source();
    src_fx_explosion = audio.create_stream_source();
    src_fx_ufo_shot = audio.create_stream_source();
    src_fx_ufo_propulsion = audio.create_stream_source();
    
    std::string font_data_path = ASCII_Fonts::get_path_to_font_data(get_exe_folder());
    std::cout << font_data_path << std::endl;
    
    auto& cs0 = color_schemes.emplace_back();
    cs0.internal.fg_color = Color::Black;
    cs0.internal.bg_color = Color::Yellow;
    auto& cs1 = color_schemes.emplace_back();
    cs1.internal.fg_color = Color::White;
    cs1.internal.bg_color = Color::Black;
    
    font_data = ASCII_Fonts::load_font_data(font_data_path);
    
    sprite_spaceship = sprh.create_vector_sprite("spaceship");
    sprite_spaceship->layer_id = 2;
    sprite_spaceship->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_spaceship->add_line_segment(0, { 1, 1 }, { -1, 0 }, 'o', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(0, { -1, 0 }, { 1, -1 }, 'o', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(0, { 0.7f, -1.f }, { 0.7f, 1.f }, '.', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(0, { 1, -1 }, { 0.7f, -1.f }, '.', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(0, { 1, 1 }, { 0.7f, 1.f }, '.', { Color::White, Color::Transparent2 }, 1);
    
    sprite_spaceship->add_line_segment(1, { 1.71f, 0 }, { 1.71f, 0 }, '*', { Color::White, Color::Transparent2 }, 2);
    sprite_spaceship->add_line_segment(1, { 1, 1 }, { -1, 0 }, 'o', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(1, { -1, 0 }, { 1, -1 }, 'o', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(1, { 0.7f, -1.f }, { 0.7f, 1.f }, '.', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(1, { 1, -1 }, { 0.7f, -1.f }, '.', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(1, { 1, 1 }, { 0.7f, 1.f }, '.', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->func_calc_anim_frame = [&](int sim_frame) { return spaceship_fwd_force > 0.f ? sim_frame % 2 : 0; };
    sprite_spaceship->set_rotation(0.f);
    sprite_spaceship->set_aspect_ratio(2.f);
    for (int frame_id = 0; frame_id < 2; ++frame_id)
    {
      sprite_spaceship->finalize_topology(frame_id);
      auto* frame = sprite_spaceship->get_curr_local_frame(frame_id);
      frame->fill_closed_polylines = false;
      frame->fill_char = '#';
      frame->fill_style = { Color::LightGray, Color::DarkGray };
    }
    rb_spaceship = dyn_sys.add_rigid_body(sprite_spaceship, 4.f,
      std::nullopt, {}, {},
      spaceship_rot_vel, 0.f,
      0.f, 0.f,
      crit_vel_r, crit_vel_c);
    rb_spaceship->set_orig_dir({ -1.f, 0.f });
    
    //   ._  ___
    //  /  \/  /
    // '       \
    // |       /
    //  \___,-'
    //
    //  /\/"/
    // (    \
    //  \___/
    //
    // ,^.^
    // (__<
    sprite_asteroid_0_big = sprh.create_bitmap_sprite("asteroid 0 big");
    sprite_asteroid_0_big->layer_id = 1;
    sprite_asteroid_0_big->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_0_big->init(5, 9);
    sprite_asteroid_0_big->create_frame(0);
    sprite_asteroid_0_big->set_sprite_chars_from_strings(0,
        "  ._  ___",
        " /  \\/  /",
        "'       \\",
        "|       /",
        " \\___,-' "
      );
    sprite_asteroid_0_big->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_0_big->fill_sprite_bg_colors(0, Color::Black);
    sprite_asteroid_0_big->fill_sprite_materials(0, 1);
    sprite_asteroid_0_big->enabled = false;
    
    sprite_asteroid_0_small = sprh.create_bitmap_sprite("asteroid 0 small");
    sprite_asteroid_0_small->layer_id = 1;
    sprite_asteroid_0_small->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_0_small->init(3, 6);
    sprite_asteroid_0_small->create_frame(0);
    sprite_asteroid_0_small->set_sprite_chars_from_strings(0,
        " /\\/\"/",
        "(    \\",
        " \\___/"
      );
    sprite_asteroid_0_small->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_0_small->fill_sprite_bg_colors(0, Color::Black);
    sprite_asteroid_0_small->fill_sprite_materials(0, 1);
    sprite_asteroid_0_small->enabled = false;
    
    sprite_asteroid_0_tiny = sprh.create_bitmap_sprite("asteroid 0 tiny");
    sprite_asteroid_0_tiny->layer_id = 1;
    sprite_asteroid_0_tiny->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_0_tiny->init(2, 4);
    sprite_asteroid_0_tiny->create_frame(0);
    sprite_asteroid_0_tiny->set_sprite_chars_from_strings(0,
        ",^.^",
        "(__<"
      );
    sprite_asteroid_0_tiny->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_0_tiny->fill_sprite_bg_colors(0, Color::Black);
    sprite_asteroid_0_tiny->fill_sprite_materials(0, 1);
    sprite_asteroid_0_tiny->enabled = false;
    
    //  ,_____
    // /      \
    // \       |
    // ,  _    |
    // \./ |__/
    //
    //  ,--.
    // (    \
    // /_/|_/
    //
    // ,-.
    // Z,_)
    sprite_asteroid_1_big = sprh.create_bitmap_sprite("asteroid 1 big");
    sprite_asteroid_1_big->layer_id = 1;
    sprite_asteroid_1_big->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_1_big->init(5, 9);
    sprite_asteroid_1_big->create_frame(0);
    sprite_asteroid_1_big->set_sprite_chars_from_strings(0,
        " ,_____  ",
        "/      \\ ",
        "\\       |",
        ",  _    |",
        "\\./ |__/ "
      );
    sprite_asteroid_1_big->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_1_big->fill_sprite_bg_colors(0, Color::Black);
    sprite_asteroid_1_big->fill_sprite_materials(0, 1);
    sprite_asteroid_1_big->enabled = false;
    
    sprite_asteroid_1_small = sprh.create_bitmap_sprite("asteroid 1 small");
    sprite_asteroid_1_small->layer_id = 1;
    sprite_asteroid_1_small->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_1_small->init(3, 6);
    sprite_asteroid_1_small->create_frame(0);
    sprite_asteroid_1_small->set_sprite_chars_from_strings(0,
        " ,--. ",
        "(    \\",
        "/_/|_/"
      );
    sprite_asteroid_1_small->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_1_small->fill_sprite_bg_colors(0, Color::Black);
    sprite_asteroid_1_small->fill_sprite_materials(0, 1);
    sprite_asteroid_1_small->enabled = false;
    
    sprite_asteroid_1_tiny = sprh.create_bitmap_sprite("asteroid 1 tiny");
    sprite_asteroid_1_tiny->layer_id = 1;
    sprite_asteroid_1_tiny->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_1_tiny->init(2, 4);
    sprite_asteroid_1_tiny->create_frame(0);
    sprite_asteroid_1_tiny->set_sprite_chars_from_strings(0,
        ",-. ",
        "Z,_)"
      );
    sprite_asteroid_1_tiny->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_1_tiny->fill_sprite_bg_colors(0, Color::Black);
    sprite_asteroid_1_tiny->fill_sprite_materials(0, 1);
    sprite_asteroid_1_tiny->enabled = false;
    
    //  -._ _.-
    // /   "  _\
    // \      \
    // /  .-._ /
    // \_/    '
    //
    //  -.-.
    // (    /
    // \_\._/
    //
    // ,v.
    // V',)
    sprite_asteroid_2_big = sprh.create_bitmap_sprite("asteroid 2 big");
    sprite_asteroid_2_big->layer_id = 1;
    sprite_asteroid_2_big->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_2_big->init(5, 9);
    sprite_asteroid_2_big->create_frame(0);
    sprite_asteroid_2_big->set_sprite_chars_from_strings(0,
        " -._ _.- ",
        "/   \"  _\\",
        "\\      \\ ",
        "/  .-._ /",
        "\\_/    ' "
      );
    sprite_asteroid_2_big->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_2_big->fill_sprite_bg_colors(0, Color::Black);
    sprite_asteroid_2_big->fill_sprite_materials(0, 1);
    sprite_asteroid_2_big->enabled = false;
    
    sprite_asteroid_2_small = sprh.create_bitmap_sprite("asteroid 2 small");
    sprite_asteroid_2_small->layer_id = 1;
    sprite_asteroid_2_small->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_2_small->init(3, 6);
    sprite_asteroid_2_small->create_frame(0);
    sprite_asteroid_2_small->set_sprite_chars_from_strings(0,
        " -.-. ",
        "(    /",
        "\\_\\._/"
      );
    sprite_asteroid_2_small->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_2_small->fill_sprite_bg_colors(0, Color::Black);
    sprite_asteroid_2_small->fill_sprite_materials(0, 1);
    sprite_asteroid_2_small->enabled = false;
    
    sprite_asteroid_2_tiny = sprh.create_bitmap_sprite("asteroid 2 tiny");
    sprite_asteroid_2_tiny->layer_id = 1;
    sprite_asteroid_2_tiny->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_2_tiny->init(2, 4);
    sprite_asteroid_2_tiny->create_frame(0);
    sprite_asteroid_2_tiny->set_sprite_chars_from_strings(0,
        ",v. ",
        "V',)"
      );
    sprite_asteroid_2_tiny->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_2_tiny->fill_sprite_bg_colors(0, Color::Black);
    sprite_asteroid_2_tiny->fill_sprite_materials(0, 1);
    sprite_asteroid_2_tiny->enabled = false;
    
    generate_big_asteroids(4);
  }
  
private:

  template<int NR, int NC>
  bool toroidal_wrap(const ScreenHandler<NR, NC>& sh, RC& pos, int c_offs, int r_offs)
  {
    if (pos.r > sh.num_rows() + r_offs)
    {
      pos.r = -r_offs;
      return true;
    }
    else if (pos.r < -r_offs)
    {
      pos.r = sh.num_rows() - 1 + r_offs;
      return true;
    }
    else if (pos.c > sh.num_cols() + c_offs)
    {
      pos.c = -c_offs;
      return true;
    }
    else if (pos.c < -c_offs)
    {
      pos.c = sh.num_cols() - 1 + c_offs;
      return true;
    }
    return false;
  }
  
  void cleanup_asteroids()
  {
    for (auto& asteroid : asteroids_vec)
    {
      sprh.remove_sprite(asteroid.sprite);
      dyn_sys.remove_rigid_body(asteroid.rb);
    }
    asteroids_vec.clear();
  }
  
  void generate_big_asteroids(int num_asteroids)
  {
    for (int a_idx = 0; a_idx < num_asteroids; ++a_idx)
    {
      Asteroid asteroid;
      std::string sprite_src_name = "asteroid " + std::to_string(rnd::rand_int(0, 2)) + " big";
      asteroid.sprite = static_cast<BitmapSprite*>(sprh.clone_sprite("asteroid big id:" + std::to_string(a_idx), sprite_src_name));
      //std::cout << asteroid.sprite->get_name() << " : " << sprite_src_name << std::endl;
      asteroid.sprite->enabled = true;
      asteroid.rb = dyn_sys.add_rigid_body(asteroid.sprite, 20.f, // mass
        Vec2 { rnd::rand_float(0.f, sh.num_rows()), rnd::rand_float(0.f, sh.num_cols()) }, // pos
        Vec2 { rnd::randn(0.f, 3.f), rnd::randn(0.f, 3.f) } // vel
      );
      asteroids_vec.emplace_back(asteroid);
    }
    coll_handler.rebuild_BVH(sh.num_rows(), sh.num_cols(), &dyn_sys);
    coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "ast");
  }

  virtual void update() override
  {
    int anim_frame = GameEngine::get_anim_count(0);
    Key curr_game_key = register_keypresses(kpdp);
    auto t = GameEngine::get_sim_time_s();
    
    // Game logic.
    
    //update_ship_controls(sh, src_fx_0, wave_gen, kpdp, curr_special_key,
    //                         get_sim_dt_s());
    
    // Auto-break velocities
    if (curr_game_key != Key::Left && curr_game_key != Key::Right)
      spaceship_rot_vel *= 0.5f;
    if (curr_game_key != Key::Thrust)
      spaceship_fwd_force = 0.f;
      
    switch (curr_game_key)
    {
      case Key::None:
        break;
      case Key::Left:
        spaceship_rot_vel = +1.5f;
        break;
      case Key::Right:
        spaceship_rot_vel = -1.5f;
        break;
      case Key::Thrust:
        spaceship_fwd_force = 7.f;
        break;
      case Key::Fire:
        if (t - shot_timestamp > shot_min_time_interval)
        {
          Shot shot;
          shot.dir = spaceship_dir;
          shot.pos = rb_spaceship->get_curr_cm() + spaceship_dir * 1.f;
          shot.time_0 = GameEngine::get_sim_time_s();
          shots_vec.emplace_back(shot);
          shot_timestamp = t;
        }
        break;
      case Key::Hyperspace:
        break;
    }
    
    // Simple Euler stepping scheme.
    auto dt = GameEngine::get_sim_dt_s();
    rb_spaceship->set_curr_ang_vel(spaceship_rot_vel);
    //spaceship_rot_ang += spaceship_rot_vel * dt;
    //sprite_spaceship->set_rotation(math::rad2deg(spaceship_rot_ang));
    spaceship_dir = rb_spaceship->get_curr_dir();
    spaceship_force = spaceship_fwd_force * spaceship_dir;
    rb_spaceship->set_curr_lin_force(spaceship_force);
    
    stlutils::erase_if(shots_vec, [&](const Shot& shot) {
      return t - shot.time_0 > shot_lifetime || shot.hit;
    });
    for (auto& shot : shots_vec)
      shot.pos += shot.dir * shot_speed * dt;
    
    // Toroidal geometry update
    auto cm = to_RC_round(rb_spaceship->get_curr_cm());
    if (toroidal_wrap(sh, cm, 0, 1))
      rb_spaceship->set_curr_cm(to_Vec2(cm));
    for (auto& shot : shots_vec)
    {
      auto pos = to_RC_round(shot.pos);
      if (toroidal_wrap(sh, pos, 0, 0))
        shot.pos = to_Vec2(pos);
    }
    for (auto& asteroid : asteroids_vec)
    {
      auto a_cm = to_RC_round(asteroid.rb->get_curr_cm());
      if (toroidal_wrap(sh, a_cm, 0, 0))
        asteroid.rb->set_curr_cm(to_Vec2(a_cm));
    }
    
    dyn_sys.update(GameEngine::get_sim_time_s(), dt, anim_frame);
    coll_handler.update();
    
    if (!stlutils::contains_if(asteroids_vec, [](const auto& a) { return !a.hit; }))
    {
      level++;
      cleanup_asteroids();
      generate_big_asteroids(level*2);
    }
    
    if (num_lives < 0)
      num_lives = 0;
    
    if (num_lives == 0)
      GameEngine::set_state_game_over();
    
    // Draw stuff.
    
    //draw_hud(sh, ...);
    
    draw_frame(sh, Color::LightGray);
    
    if (dbg_draw_rigid_bodies)
      dyn_sys.draw_dbg(sh);
    if (dbg_draw_sprites)
      sprh.draw_dbg_pts(sh, anim_frame);
    if (dbg_draw_narrow_phase)
      coll_handler.draw_dbg_narrow_phase(sh);
    if (draw_sprites)
      sprh.draw(sh, anim_frame);
    if (dbg_draw_sprites)
      sprh.draw_dbg_bb(sh, anim_frame);
    if (dbg_draw_broad_phase)
      coll_handler.draw_dbg_broad_phase(sh, 0);
      
    for (const auto& shot : shots_vec)
      sh.write_buffer(".", std::round(shot.pos.r), std::round(shot.pos.c), Color::White);
  }
  
  virtual void on_quit() override
  {
    chip_tune.stop_tune_async();
  }
  
  virtual void draw_title() override
  {
    ::draw_title(sh, font_data, color_schemes[0]);
  }
  
  virtual void draw_instructions() override
  {
    ::draw_instructions(sh, font_data, color_schemes[1]);
  }
  
  virtual void on_exit_instructions() override
  {
    chip_tune.stop_tune_async();
  }

  //////////////////////////////////////////////////////////////////////////
  
  audio::AudioSourceHandler audio;
  audio::WaveformGeneration wave_gen;
  audio::ChipTuneEngine chip_tune { audio, wave_gen };
  audio::AudioStreamSource* src_fx_shot = nullptr;
  audio::AudioStreamSource* src_fx_explosion = nullptr;
  audio::AudioStreamSource* src_fx_ufo_shot = nullptr;
  audio::AudioStreamSource* src_fx_ufo_propulsion = nullptr;
  
  std::vector<ASCII_Fonts::ColorScheme> color_schemes;
  ASCII_Fonts::FontDataColl font_data;
  
  SpriteHandler sprh;
  dynamics::DynamicsSystem dyn_sys;
  dynamics::CollisionHandler coll_handler;
  bool dbg_draw_rigid_bodies = false;
  bool dbg_draw_sprites = false;
  bool dbg_draw_narrow_phase = false;
  bool dbg_draw_broad_phase = false;
  bool draw_sprites = true;
  
  int num_lives = 10;
  
  VectorSprite* sprite_spaceship = nullptr;
  dynamics::RigidBody* rb_spaceship = nullptr;
  
  float spaceship_rot_vel = 0.f;
  //float spaceship_rot_ang = 0.f;
  float spaceship_fwd_force = 0.f;
  Vec2 spaceship_force { 0.f, 0.f };
  Vec2 spaceship_dir { -1.f, 0.f };
  float crit_vel_c = 30.f;
  float crit_vel_r = crit_vel_c/1.5f;
  
  float shot_speed = 31.f;
  float shot_lifetime = 2.f;
  float shot_min_time_interval = 0.1f; // Minimum time allowed between shots.
  float shot_timestamp = 0.f;
  struct Shot
  {
    Vec2 dir;
    Vec2 pos;
    bool hit = false;
    float time_0 = 0.f;
  };
  std::vector<Shot> shots_vec;
  
  BitmapSprite* sprite_asteroid_0_big = nullptr;
  BitmapSprite* sprite_asteroid_1_big = nullptr;
  BitmapSprite* sprite_asteroid_2_big = nullptr;
  BitmapSprite* sprite_asteroid_0_small = nullptr;
  BitmapSprite* sprite_asteroid_1_small = nullptr;
  BitmapSprite* sprite_asteroid_2_small = nullptr;
  BitmapSprite* sprite_asteroid_0_tiny = nullptr;
  BitmapSprite* sprite_asteroid_1_tiny = nullptr;
  BitmapSprite* sprite_asteroid_2_tiny = nullptr;
  
  struct Asteroid
  {
    BitmapSprite* sprite = nullptr;
    dynamics::RigidBody* rb = nullptr;
    Asteroid* child_A = nullptr;
    Asteroid* child_B = nullptr;
    int level = 0; // 0 : big, 1 : small, 2 : tiny.
    bool hit = false;
  };
  std::vector<Asteroid> asteroids_vec;
  
  int level = 2;
};

//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  GameEngineParams params;
  params.screen_bg_color_default = Color::Black;
  params.screen_bg_color_title = Color::Black;
  params.screen_bg_color_instructions = Color::Black;
  params.enable_title_screen = false;
  params.enable_instructions_screen = false;
  
  Game game(argc, argv, params);

  if (argc >= 2 && strcmp(argv[1], "--help") == 0)
  {
    std::cout << "asciiroids (\"--help\" | [<frame-delay-us>])" << std::endl;
    std::cout << "  default values:" << std::endl;
    std::cout << "    <frame-delay-us>    : " << game.get_sim_delay_us() << std::endl;
    return EXIT_SUCCESS;
  }

  game.init();
  game.generate_data();
  game.run();

  return EXIT_SUCCESS;
}
