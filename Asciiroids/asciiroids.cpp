//
//  asciiroids.cpp
//  Asciiroids
//
//  Created by Rasmus Anthin on 2025-07-05.
//

#include "Keyboard.h"
#include "TitleScreen.h"
#include "InstructionsScreen.h"

#include <Core/Timer.h>
#include <Termin8or/GameEngine.h>
#include <Termin8or/SpriteHandler.h>
#include <Termin8or/ASCII_Fonts.h>
#include <Termin8or/Dynamics/RigidBody.h>
#include <Termin8or/Dynamics/DynamicsSystem.h>
#include <Termin8or/Dynamics/CollisionHandler.h>
#include <8Beat/AudioSourceHandler.h>
#include <8Beat/ChipTuneEngine.h>
#include <8Beat/WaveformGeneration.h>
#include <8Beat/SFX.h>

#include <fstream>

//#define DESIGN_SFX

// ////////////////////////////
// [x] Explosion sprites.
// [x] Spaceship collision logic (explosion + reappearance, etc).
// [x] Score counting.
// [x] Shots should split larger asteroids into two smaller ones which travel faster than the original.
// [ ] Hyperspace.
// [ ] Large UFO.
// [ ] Small UFO.
// [ ] UFOs shoot at spaceship.
// [ ] Spaceship can shoot at UFOs.
// [x] SFX.
// [x] Music.
// ////////////////////////////

class Game : public GameEngine<40, 100>
{
#ifdef DESIGN_SFX
  std::vector<float> vp_design;
  int channel = 0;
#endif

public:
  Game(int argc, char** argv, const GameEngineParams& params)
    : GameEngine(argv[0], params)
  {
  //#ifndef _WIN32
    GameEngine::set_real_fps(15);
    GameEngine::set_sim_delay_us(50'000);
    GameEngine::set_anim_rate(0, 4); // Explosion
    GameEngine::set_anim_rate(1, 3); // Asteroids
    GameEngine::set_anim_rate(2, 5); // UFO AI
  //#endif
  
    shot_timer.set(0.f);
  
    if (argc >= 2 && strcmp(argv[1], "-") != 0)
      GameEngine::set_real_fps(static_cast<float>(atoi(argv[1])));
  }
  
  ~Game()
  {
    if (src_fx_shot != nullptr)
      audio.remove_source(src_fx_shot);
    if (src_fx_explosion != nullptr)
      audio.remove_source(src_fx_explosion);
    if (src_fx_ufo_shot != nullptr)
      audio.remove_source(src_fx_ufo_shot);
    if (src_fx_ufo_propulsion != nullptr)
      audio.remove_source(src_fx_ufo_propulsion);
  }

  virtual void generate_data() override
  {
#ifndef DESIGN_SFX
    try
    {
      std::string tune_path = get_exe_folder();
#ifndef _WIN32
      const char* xcode_env = std::getenv("RUNNING_FROM_XCODE");
      if (xcode_env != nullptr)
        tune_path = "../../../../../../../../Documents/xcode/Asciiroids/Asciiroids/"; // #FIXME: Find a better solution!
#endif
    
      if (chip_tune.load_tune(folder::join_path({ tune_path, "music.ct" })))
      {
          chip_tune.set_volume(0.2f);
          //chip_tune.play_tune();
          chip_tune.play_tune_async();
          chip_tune.wait_for_completion();
      }
    }
    catch (const std::exception& e)
    {
      std::cerr << "Caught exception: " << e.what() << std::endl;
    }
#endif
    
    {
      using namespace audio;
#ifdef DESIGN_SFX
      vp_design.resize(17);
#endif
      static std::vector<float> vp_shot
      {
        0.32f,
        0.27f,
        0.f,
        -0.16f,
        -0.11f,
        0.2f,
        0.f,
        0.f,
        0.5f,
        0.31f,
        0.7f,
        0.f,
        0.f,
        0.f,
        0.f,
        0.f,
        0.f,
      };
      static std::vector<float> vp_explosion
      {
        -1.5f,
        -1.f,
        0.5f,
        -0.8f,
        2.f,
        0.f,
        1.5f,
        0.7f,
        0.05f,
        0.5f,
        -0.5f,
        1.0f,
        0.f,
        0.f,
        0.f,
        0.f,
        0.f,
      };
      src_fx_shot = audio.create_stream_source();
      auto wd_shot = SFX::generate(SFXType::LASER, vp_shot);
      src_fx_shot->update_buffer(wd_shot);
      src_fx_shot->set_volume(0.25f);
      src_fx_explosion = audio.create_stream_source();
      auto wd_explosion = SFX::generate(SFXType::EXPLOSION, vp_explosion);
      src_fx_explosion->update_buffer(wd_explosion);
      src_fx_explosion->set_volume(1.f);
      src_fx_ufo_shot = audio.create_stream_source();
      src_fx_ufo_propulsion = audio.create_stream_source();
    }
    
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
    sprite_spaceship->set_aspect_ratio(spaceship_ar);
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
    
    Color asteroid_bg_color = use_transparent_asteroids ? Color::Transparent2 : Color::Black;
    
    // +-----------+
    // |   ._  ___ |
    // |  /  \/  / |
    // | '       \ |
    // | |       / |
    // |  \___,-'  |
    // +--------+--+
    // |  /\/"/ |
    // | (    \ |
    // |  \___/ |
    // +------+-+
    // | ,^.^ |
    // | (__< |
    // +------+
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
    sprite_asteroid_0_big->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_0_big->set_sprite_materials(0,
        0, 0, 1, 1, 0, 0, 1, 1, 1,
        0, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 1, 1, 1, 1, 1, 1, 1, 0
      );
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
    sprite_asteroid_0_small->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_0_small->set_sprite_materials(0,
        0, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1,
        0, 1, 1, 1, 1, 1
      );
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
    sprite_asteroid_0_tiny->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_0_tiny->fill_sprite_materials(0, 1);
    sprite_asteroid_0_tiny->enabled = false;
    
    // +-----------+
    // |  ,_____   |
    // | /      \  |
    // | \       | |
    // | ,  _    | |
    // | \./ |__/  |
    // +--------+--+
    // |  ,--.  |
    // | (    \ |
    // | /_/|_/ |
    // +------+-+
    // | ,-.  |
    // | Z,_) |
    // +------+
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
    sprite_asteroid_1_big->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_1_big->set_sprite_materials(0,
        0, 1, 1, 1, 1, 1, 1, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 0
      );
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
    sprite_asteroid_1_small->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_1_small->set_sprite_materials(0,
        0, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1
      );
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
    sprite_asteroid_1_tiny->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_1_tiny->set_sprite_materials(0,
        1, 1, 1, 0,
        1, 1, 1, 1
      );
    sprite_asteroid_1_tiny->enabled = false;
    
    // +-----------+
    // |  -._ _.-  |
    // | /   "  _\ |
    // | \      \  |
    // | /  .-._ / |
    // | \_/    '  |
    // +--------+--+
    // |  -.-.  |
    // | (    / |
    // | \_\._/ |
    // +------+-+
    // | ,v.  |
    // | V',) |
    // +------+
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
    sprite_asteroid_2_big->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_2_big->set_sprite_materials(0,
        0, 1, 1, 1, 0, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 0, 0, 0, 0, 1, 0
      );
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
    sprite_asteroid_2_small->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_2_small->set_sprite_materials(0,
        0, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1
      );
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
    sprite_asteroid_2_tiny->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_2_tiny->set_sprite_materials(0,
        1, 1, 1, 0,
        1, 1, 1, 1
      );
    sprite_asteroid_2_tiny->enabled = false;
    
    generate_big_asteroids(4);
    
    sprite_explosion = sprh.create_bitmap_sprite("explosion");
    sprite_explosion->layer_id = 3;
    sprite_explosion->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_explosion->init(5, 9);
    sprite_explosion->enabled = false;
    sprite_explosion->create_frame(0);
    sprite_explosion->set_sprite_chars_from_strings(0,
        "         ",
        "         ",
        "    .    ",
        "         ",
        "         "
      );
    sprite_explosion->fill_sprite_fg_colors(0, Color::White);
    sprite_explosion->fill_sprite_bg_colors(0, Color::Transparent2);
    sprite_explosion->create_frame(1);
    sprite_explosion->set_sprite_chars_from_strings(1,
        "         ",
        "    ..   ",
        "   . ..  ",
        "    .    ",
        "         "
      );
    sprite_explosion->fill_sprite_fg_colors(1, Color::White);
    sprite_explosion->fill_sprite_bg_colors(1, Color::Transparent2);
    sprite_explosion->create_frame(2);
    sprite_explosion->set_sprite_chars_from_strings(2,
        "    . .  ",
        "  .  . . ",
        " . .  .  ",
        "  .    . ",
        "    .    "
      );
    sprite_explosion->fill_sprite_fg_colors(2, Color::White);
    sprite_explosion->fill_sprite_bg_colors(2, Color::Transparent2);
    sprite_explosion->create_frame(3);
    sprite_explosion->set_sprite_chars_from_strings(3,
        " . .. .  ",
        "  .  . . ",
        " ..    . ",
        "  .   .  ",
        " .  .   ."
      );
    sprite_explosion->fill_sprite_fg_colors(3, Color::White);
    sprite_explosion->fill_sprite_bg_colors(3, Color::Transparent2);
    sprite_explosion->create_frame(4);
    sprite_explosion->set_sprite_chars_from_strings(4,
        ".   .  . ",
        " .     . ",
        ".       .",
        "         ",
        " .  .   ."
      );
    sprite_explosion->fill_sprite_fg_colors(4, Color::White);
    sprite_explosion->fill_sprite_bg_colors(4, Color::Transparent2);
    sprite_explosion->create_frame(5);
    sprite_explosion->set_sprite_chars_from_strings(5,
        "         ",
        "         ",
        ".        ",
        "         ",
        "     .   "
      );
    sprite_explosion->fill_sprite_fg_colors(5, Color::White);
    sprite_explosion->fill_sprite_bg_colors(5, Color::Transparent2);
    sprite_explosion->create_frame(6);
    sprite_explosion->set_sprite_chars_from_strings(6,
        "         ",
        "         ",
        "         ",
        "         ",
        "         "
      );
    sprite_explosion->fill_sprite_fg_colors(6, Color::White);
    sprite_explosion->fill_sprite_bg_colors(6, Color::Transparent2);
    
    sprite_ufo_large = sprh.create_bitmap_sprite("ufo large");
    sprite_ufo_large->enabled = false;
    sprite_ufo_large->layer_id = 1;
    sprite_ufo_large->pos = { 2, sh.num_cols() - 2 };
    sprite_ufo_large->init(2, 5);
    sprite_ufo_large->create_frame(0);
    sprite_ufo_large->fill_sprite_fg_colors(0, Color::White);
    sprite_ufo_large->fill_sprite_bg_colors(0, Color::Transparent2);
    sprite_ufo_large->set_sprite_materials(0,
      0, 1, 1, 1, 0,
      1, 1, 1, 1, 1);
    sprite_ufo_large->set_sprite_chars_from_strings(0,
      " /-\\ ",
      "<--->"
    );

    sprite_ufo_small = sprh.create_bitmap_sprite("ufo small");
    sprite_ufo_small->enabled = false;
    sprite_ufo_small->layer_id = 1;
    sprite_ufo_small->pos = { 2, sh.num_cols() - 2 };
    sprite_ufo_small->init(1, 3);
    sprite_ufo_small->create_frame(0);
    sprite_ufo_small->fill_sprite_fg_colors(0, Color::White);
    sprite_ufo_small->fill_sprite_bg_colors(0, Color::Transparent2);
    sprite_ufo_small->fill_sprite_materials(0, 1);
    sprite_ufo_small->set_sprite_chars_from_strings(0,
      "<^>"
    );
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
      auto& asteroid = asteroids_vec.emplace_back();
      std::string sprite_src_name = "asteroid " + std::to_string(rnd::rand_int(0, 2)) + " big";
      asteroid.sprite = static_cast<BitmapSprite*>(sprh.clone_sprite("asteroid big id:" + std::to_string(global_asteroid_id++), sprite_src_name));
      //std::cout << asteroid.sprite->get_name() << " : " << sprite_src_name << std::endl;
      asteroid.sprite->enabled = true;
      auto nr_f = static_cast<float>(sh.num_rows());
      auto nc_f = static_cast<float>(sh.num_cols());
      Vec2 pos;
      for (;;)
      {
        pos = Vec2 { rnd::rand_float(0.f, nr_f), rnd::rand_float(0.f, nc_f) };
        if (math::distance_squared_ar(pos, rb_spaceship->get_curr_cm(), 2.f) > c_min_ship_asteroid_dist_sq)
          break;
      }
      asteroid.rb = dyn_sys.add_rigid_body(asteroid.sprite, 20.f, // mass
        pos, // pos
        Vec2 { rnd::randn(0.f, 3.f), rnd::randn(0.f, 3.f) } // vel
      );
    }
    coll_handler.rebuild_BVH(sh.num_rows(), sh.num_cols(), &dyn_sys);
    coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "ast");
  }
  
  template<int NR, int NC>
  void draw_hud(ScreenHandler<NR, NC>& sh)
  {
    sh.write_buffer(str::adjust_str(std::to_string(GameEngine::ref_score()), str::Adjustment::Right, 6), 1, 8, Color::White);
  
    for (int life_idx = 0; life_idx < std::min(num_lives, 10); ++life_idx)
    {
      sh.write_buffer("A", 2, 10 + life_idx, Color::White);
    }
  }
  
  // ///////

  virtual void update() override
  {
#ifdef DESIGN_SFX
    {
      using namespace audio;
      
      auto key_held = keyboard::get_char_key(kpdp.held);
      auto special_key = keyboard::get_special_key(kpdp.held);
      for (int i = 0; i < 17; ++i)
      {
        int j_max = math::linmap(vp_design[i], -2.f, 3.f, 0, 50);
        for (int j = 0; j < j_max; ++j)
          sh.write_buffer("#", i+2, j+15, channel == i ? Color::Yellow : Color::White);
        sh.write_buffer(std::to_string(vp_design[i]), i+2, 5, Color::Cyan);
        sh.write_buffer(std::to_string(i) + '.', i+2, 1, Color::Blue);
      }
      
      auto key = keyboard::get_special_key(kpdp.transient);
      switch (key)
      {
        case keyboard::SpecialKey::Up:
          channel--;
          if (channel < 0)
            channel = 16;
          break;
        case keyboard::SpecialKey::Down:
          channel++;
          if (channel > 16)
            channel = 0;
          break;
        case keyboard::SpecialKey::Left:
          vp_design[channel] -= 0.01f;
          if (vp_design[channel] < -2.f)
            vp_design[channel] = -2.f;
          break;
        case keyboard::SpecialKey::Right:
          vp_design[channel] += 0.01f;
          if (vp_design[channel] > 3.f)
            vp_design[channel] = 3.f;
          break;
        default:
          break;
      }
      
      if (keyboard::get_char_key(kpdp.transient) == ' ')
      {
        auto wd_shot = SFX::generate(SFXType::LASER, vp_design);
        src_fx_shot->update_buffer(wd_shot);
      
        src_fx_shot->play();
      }
    }
#else
  
    // Game logic.
    int anim_frame = GameEngine::get_anim_count(0);
    auto t = GameEngine::get_sim_time_s();
    Key curr_game_key = register_keypresses(kpdp);
    
    //update_ship_controls(sh, src_fx_0, wave_gen, kpdp, curr_special_key,
    //                         get_sim_dt_s());
    
    // Auto-break velocities
    if (curr_game_key != Key::Left && curr_game_key != Key::Right)
      spaceship_rot_vel *= 0.5f;
    if (curr_game_key != Key::Thrust)
      spaceship_fwd_force = 0.f;
    
    if (sprite_spaceship->enabled)
    {
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
          spaceship_fwd_force = 10.f; //7.f;
          break;
        case Key::Fire:
          if (shot_timer.wait(t))
          {
            Shot shot;
            shot.dir = Vec2 { spaceship_dir.r / spaceship_ar, spaceship_dir.c };
            shot.dir = math::normalize(shot.dir);
            shot.pos = rb_spaceship->get_curr_cm() + spaceship_dir * 1.f;
            shot.time_0 = GameEngine::get_sim_time_s();
            shots_vec.emplace_back(shot);
            shot_timer.set(t);
            src_fx_shot->play();
          }
          break;
        case Key::Hyperspace:
          if (hyperspace_jump_timer.set(t))
          {
            sprite_spaceship->enabled = false;
            coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "spa");
          }
          break;
      }
    }
    
    score_prev = GameEngine::ref_score();
    
    // Simple Euler stepping scheme.
    auto dt = GameEngine::get_sim_dt_s();
    rb_spaceship->set_curr_ang_vel(spaceship_rot_vel);
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
    
    // Handle collisions and generate explosions:
    //  * asteroid <-> spaceship
    //  * small ufo <-> spaceship
    //  * large ufo <-> spaceship
    auto f_generate_explosion = [&](const RC& pos) -> ExplosionData*
    {
      auto& explosion = explosions_vec.emplace_back(std::make_unique<ExplosionData>());
      explosion->trig = true;
      explosion->sprite = static_cast<BitmapSprite*>(sprh.clone_sprite("explosion " + std::to_string(global_explosion_id++), "explosion"));
      explosion->sprite->enabled = true;
      explosion->sprite->pos = pos - sprite_explosion->get_size() / 2;
      auto* expl_raw_ptr = explosion.get();
      explosion->sprite->func_calc_anim_frame = [expl_raw_ptr](int sim_frame) { return expl_raw_ptr->anim_ctr; };
      src_fx_explosion->play();
      return expl_raw_ptr;
    };
    auto isect_data = coll_handler.get_isect_world_positions();
    for (const auto& id : isect_data)
    {
      if (!stlutils::contains_if(explosions_vec, [&id](const auto& expl) { return (id.node_A == expl->isect_data.node_A && id.node_B == expl->isect_data.node_B) || (id.node_A == expl->isect_data.node_B && id.node_B == expl->isect_data.node_A); }))
      {
        auto* explosion = f_generate_explosion(to_RC_round(id.world_pos));
        explosion->isect_data = id;
      }
    }

    // Split asteroids when shot upon.
    new_asteroids_vec.clear();
    for (auto& asteroid : asteroids_vec)
    {
      auto aabb_asteroid = asteroid.sprite->calc_curr_AABB(0);
      RC hit_rc;
      for (auto& shot : shots_vec)
      {
        auto shot_rc = to_RC_round(shot.pos);
        if (aabb_asteroid.contains(shot_rc))
        {
          asteroid.hit = true;
          shot.hit = true;
          hit_rc = shot_rc;
          break;
        }
      }
      stlutils::erase_if(shots_vec, [](const auto& s) { return s.hit; });
      if (asteroid.hit)
      {
        f_generate_explosion(hit_rc);
        
        if (asteroid.level <= 1)
        {
          Asteroid a_child[2];
          Vec2 a_child_dir[2];
          const float ar = 1.8f;
          a_child_dir[0] = Vec2 { rnd::rand_float(-1.f, +1.f)/ar, rnd::rand_float(-1.f, +1.f) };
          a_child_dir[0] = math::normalize(a_child_dir[0]);
          a_child_dir[1] = Vec2 {
            -a_child_dir[0].r + rnd::randn(0.f, 0.2f)/ar,
            -a_child_dir[0].c + rnd::randn(0.f, 0.2f)
          };
          a_child_dir[1] = math::normalize(a_child_dir[1]);
          for (int a_idx = 0; a_idx < 2; ++a_idx)
          {
            a_child[a_idx].level = asteroid.level + 1;
            std::string a_size = a_child[a_idx].level == 1 ? "small" : "tiny";
            std::string sprite_src_name = "asteroid " + std::to_string(rnd::rand_int(0, 2)) + " " + a_size;
            a_child[a_idx].sprite = static_cast<BitmapSprite*>(sprh.clone_sprite("asteroid " + a_size + " id:" + std::to_string(global_asteroid_id++), sprite_src_name));
            //std::cout << a_child[a_idx].sprite->get_name() << " : " << sprite_src_name << std::endl;
            a_child[a_idx].sprite->enabled = true;
            auto pos = asteroid.rb->get_curr_cm();
            pos.r += a_child_dir[a_idx].r * rnd::rand_float(0.1f, 3.f)/ar;
            pos.c += a_child_dir[a_idx].c * rnd::rand_float(0.1f, 3.f);
            auto vel = a_child_dir[a_idx] * (math::length(asteroid.rb->get_curr_lin_vel()) + rnd::rand_float(0.05f, 3.f));
            a_child[a_idx].rb = dyn_sys.add_rigid_body(a_child[a_idx].sprite, 20.f, // mass
              pos, vel);
            new_asteroids_vec.emplace_back(a_child[a_idx]);
          }
        }
        
        sprh.remove_sprite(asteroid.sprite);
        dyn_sys.remove_rigid_body(asteroid.rb);
        switch (asteroid.level)
        {
          case 0: GameEngine::ref_score() += 20; break;
          case 1: GameEngine::ref_score() += 50; break;
          case 2: GameEngine::ref_score() += 100; break;
        }
      }
    }
    stlutils::erase_if(asteroids_vec, [](const auto& a) { return a.hit; });
    stlutils::append(asteroids_vec, new_asteroids_vec);
    if (!new_asteroids_vec.empty())
    {
      coll_handler.rebuild_BVH(sh.num_rows(), sh.num_cols(), &dyn_sys);
      coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "ast");
    }
    
    // If spaceship collided with something then lose a life and make the ship disappear and reappear in a safe zone.
    auto id_it = stlutils::find_if(isect_data, [&](const auto& id) { return id.node_A->rigid_body == rb_spaceship || id.node_B->rigid_body == rb_spaceship; });
    if (id_it != isect_data.end() && spaceship_reappearance_timer.set(t))
    {
      sprite_spaceship->enabled = false;
      coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "spa");
      num_lives--;
    }
    if (spaceship_reappearance_timer.wait(t, false))
    {
      const auto& spaceship_pos = rb_spaceship->get_curr_cm();
      if (!stlutils::contains_if(asteroids_vec, [&spaceship_pos, this](const auto& a) { return math::distance_squared_ar(a.rb->get_curr_cm(), spaceship_pos, 2.f) < c_min_ship_asteroid_dist_sq; }))
      {
        spaceship_reappearance_timer.reset();
        sprite_spaceship->enabled = true;
        rb_spaceship->set_curr_cm({ sh.num_rows()/2.f, sh.num_cols()/2.f });
        rb_spaceship->set_curr_lin_vel({ 0, 0 });
        rb_spaceship->set_curr_ang(0.f);
        coll_handler.reinclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "spa");
      }
    }
    
    // Hyperspace jump destination.
    if (hyperspace_jump_timer.wait(t, false))
    {
      auto nr_f = static_cast<float>(sh.num_rows());
      auto nc_f = static_cast<float>(sh.num_cols());
      Vec2 pos;
      int iter = 0;
      for (;;)
      {
        pos = Vec2 { rnd::rand_float(0.f, nr_f), rnd::rand_float(0.f, nc_f) };
        bool too_close = false;
        for (const auto& asteroid : asteroids_vec)
        {
          if (math::distance_squared_ar(pos, asteroid.rb->get_curr_cm(), 2.f) < c_min_ship_asteroid_dist_sq)
            too_close = true;
        }
        if (!too_close)
        {
          hyperspace_jump_timer.reset();
          rb_spaceship->set_curr_cm(pos);
          rb_spaceship->set_curr_lin_vel({ 0, 0 });
          sprite_spaceship->enabled = true;
          coll_handler.reinclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "spa");
          break;
        }
        if (iter++ > 5) // Making sure we don't spend too much time here and stall the main thread.
          break;
      }
    }
    
    // UFOs
    if (ufo_active_timer.is_ticking(t))
    {
      Vec2 pos;
      if (sprite_ufo_large->enabled)
      {
        pos = rb_ufo_large->get_curr_cm();
        rb_ufo_large->set_curr_lin_vel({ 0.f, 0.f });
      }
      else if (sprite_ufo_small->enabled)
      {
        pos = rb_ufo_small->get_curr_cm();
        rb_ufo_small->set_curr_lin_vel({ 0.f, 0.f });
      }
      else
      {
        std::cerr << "ERROR : An UFO was expected to be active but none was found!\n";
      }
      
      if (ufo_h_dir == +1)
        pos.c += ufo_delta_pos;
      else if (ufo_h_dir == -1)
        pos.c -= ufo_delta_pos;
        
      if (ufo_v_dir == +1)
        pos.r -= ufo_delta_pos/2.5f;
      else if (ufo_v_dir == -1)
        pos.r += ufo_delta_pos/2.5f;
        
      auto rc = to_RC_round(pos);
      if (toroidal_wrap(sh, rc, 0, 0))
        pos = to_Vec2(rc);
        
      if (sprite_ufo_large->enabled)
        rb_ufo_large->set_curr_cm(pos);
      else if (sprite_ufo_small->enabled)
        rb_ufo_small->set_curr_cm(pos);
        
      if (!ufo_v_move_timer.is_ticking(t) && rnd::one_in(50))
      {
        ufo_v_dir = rnd::rand_int(-1, +1);
        ufo_v_move_timer.set(t);
        if (ufo_v_dir == 0)
          ufo_v_move_timer.set_delay(rnd::randn_clamp(3.f, 1.f, 0.5f, 5.f));
        else
          ufo_v_move_timer.set_delay(rnd::randn_clamp(1.5f, 1.f, 0.5f, 5.f));
      }
    }
    else
    {
      // Finished.
      if (ufo_trig.once())
      {
        ufo_active_timer.reset();
        sprite_ufo_large->enabled = false;
        sprite_ufo_small->enabled = false;
        dyn_sys.remove_rigid_body(rb_ufo_large);
        dyn_sys.remove_rigid_body(rb_ufo_small);
      }
      
      // Respawn.
      auto f_set_ufo_pos = [this](Vec2& pos, int& h_dir) -> bool
      {
        int iters = 0;
        auto nr_f = static_cast<float>(sh.num_rows());
        auto nc_f = static_cast<float>(sh.num_cols());
        for (;;)
        {
          pos.r = rnd::rand_float(0.f, nr_f);
          if (rnd::one_in(2))
          {
            // Left -> Right
            pos.c = rnd::rand_float(0.f, nc_f/3.f);
            h_dir = +1;
          }
          else
          {
            // Right -> Left
            pos.c = rnd::rand_float(2.f/3.f*nc_f, nc_f);
            h_dir = -1;
          }
          if (math::distance_squared_ar(pos, rb_spaceship->get_curr_cm(), 2.f) > c_min_ship_asteroid_dist_sq)
            return true;
          if (iters++ > 100)
            break;
        }
        return false;
      };
      Vec2 pos;
      if (rnd::one_in(100) && f_set_ufo_pos(pos, ufo_h_dir))
      {
        ufo_active_timer.set(t);
        ufo_trig.reset();
        sprite_ufo_large->enabled = true;
        rb_ufo_large = dyn_sys.add_rigid_body(sprite_ufo_large, 4.f, pos);
        rb_ufo_large->set_orig_dir({ -1.f, 0.f });
      }
      else if (rnd::one_in(100) && f_set_ufo_pos(pos, ufo_h_dir))
      {
        ufo_active_timer.set(t);
        ufo_trig.reset();
        sprite_ufo_small->enabled = true;
        rb_ufo_small = dyn_sys.add_rigid_body(sprite_ufo_small, 2.f, pos);
        rb_ufo_small->set_orig_dir({ -1.f, 0.f });
      }
    }
    
    // Handling of explosions lifetimes.
    for (auto& explosion : explosions_vec)
    {
      if (explosion->trig)
      {
        explosion->timestamp = GameEngine::get_anim_count(0);
        explosion->anim_ctr = 0;
        explosion->trig = false;
      }
      else if (explosion->anim_ctr < 6)
        explosion->anim_ctr = GameEngine::get_anim_count(0) - explosion->timestamp;
      else
        explosion->sprite->enabled = false;
    }
    stlutils::erase_if(explosions_vec, [&](auto& expl)
    {
      if (!expl->sprite->enabled)
      {
        sprh.remove_sprite(expl->sprite);
        return true;
      }
      return false;
    });
    
    // Level logic.
    if (asteroids_vec.empty() && level_timer.set(t))
    {
      cleanup_asteroids();
      chip_tune.stop_tune_async();
    }
    else if (level_timer.wait(t))
    {
      level++;
      generate_big_asteroids(level*2);
      chip_tune.play_tune_async();
    }
    
    // Extra life every 10'000 points.
    if (GameEngine::ref_score() - score_prev)
    {
      auto diff_score = GameEngine::ref_score()/10'000 - score_prev/10'000;
      if (diff_score > 0)
        num_lives += diff_score;
    }
    
    // Game Over if no more lives.
    if (num_lives < 0)
      num_lives = 0;
    if (num_lives == 0)
      GameEngine::set_state_game_over();
      
    // Update dynamics and collisions.
    dyn_sys.update(GameEngine::get_sim_time_s(), dt, anim_frame);
    coll_handler.update();
    
    // Draw stuff.
    
    draw_hud(sh);
    
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
      sh.write_buffer(".", math::roundI(shot.pos.r), math::roundI(shot.pos.c), Color::White);
#endif
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
    //chip_tune.stop_tune_async();
  }
  
  virtual void on_enter_paused() override
  {
    chip_tune.pause();
  }
  
  virtual void on_exit_paused() override
  {
    chip_tune.resume();
  }
  
  virtual void on_enter_game_over() override
  {
    chip_tune.stop_tune_async();
  }
  
  virtual void on_enter_input_hiscore() override
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
  
  int num_lives = 3; // max visible lives : 10, but more lives can be stored. You gain an extra life for every 10'000 points gained.
  int score_prev = 0; // max score : 999990
  int level = 2;
  Timer level_timer { 2.f };
  const float c_min_ship_asteroid_dist_sq = math::sq(10.f);
  
  VectorSprite* sprite_spaceship = nullptr;
  dynamics::RigidBody* rb_spaceship = nullptr;
  
  const float spaceship_ar = 2.f;
  float spaceship_rot_vel = 0.f;
  float spaceship_fwd_force = 0.f;
  Vec2 spaceship_force { 0.f, 0.f };
  Vec2 spaceship_dir { -1.f, 0.f };
  float crit_vel_c = 30.f;
  float crit_vel_r = crit_vel_c/1.5f;
  Timer spaceship_reappearance_timer { 0.2f };
  Timer hyperspace_jump_timer { 1.5f };
  
  float shot_speed = 31.f;
  float shot_lifetime = 2.f;
  Timer shot_timer { 0.1f }; // Minimum time allowed between shots.
  struct Shot
  {
    Vec2 dir;
    Vec2 pos;
    bool hit = false;
    float time_0 = 0.f;
  };
  std::vector<Shot> shots_vec;
  
  bool use_transparent_asteroids = true;
  
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
    int level = 0; // 0 : big, 1 : small, 2 : tiny.
    bool hit = false;
  };
  int global_asteroid_id = 0;
  std::vector<Asteroid> asteroids_vec;
  std::vector<Asteroid> new_asteroids_vec;
  
  BitmapSprite* sprite_explosion = nullptr;
  //VectorSprite* sprite_broken_ship = nullptr;
  
  struct ExplosionData
  {
    bool trig = false;
    int timestamp = 0;
    int anim_ctr = 0;
    dynamics::CollisionHandler::IsectData isect_data;
    BitmapSprite* sprite = nullptr;
  };
  int global_explosion_id = 0;
  std::vector<std::unique_ptr<ExplosionData>> explosions_vec;
  
  BitmapSprite* sprite_ufo_large = nullptr;
  BitmapSprite* sprite_ufo_small = nullptr;
  dynamics::RigidBody* rb_ufo_large = nullptr;
  dynamics::RigidBody* rb_ufo_small = nullptr;
  Timer ufo_active_timer { 7.f };
  Timer ufo_v_move_timer { 2.f }; // 2s vertical travel.
  OneShot ufo_trig;
  Vec2 ufo_shot_dir;
  // UFO Motion e.g.
  //      /----\
  // ----/      \
  //             \----/
  int ufo_h_dir = -1; // -1 : left, +1 : right.
  int ufo_v_dir = 0; // -1 : down, 0 : unchanged, +1 : up.
  float ufo_delta_pos = 0.5f;
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
  params.pause_info_style = { Color::White, Color::Transparent };
  params.screen_bg_color_quit_confirm = std::nullopt;
  params.quit_confirm_title_style = { Color::LightGray, Color::Transparent };
  params.quit_confirm_button_style = { Color::Black, Color::DarkGray, Color::LightGray };
  params.quit_confirm_info_style = { Color::DarkGray, Color::Transparent };
  params.screen_bg_color_input_hiscore = std::nullopt;
  params.input_hiscore_title_style = { Color::LightGray, Color::Transparent };
  params.input_hiscore_prompt_style = { Color::White, Color::DarkGray, Color::LightGray };
  params.input_hiscore_info_style = { Color::DarkGray, Color::Transparent };
  params.screen_bg_color_hiscores = std::nullopt;
  params.hiscores_title_style = { Color::LightGray, Color::DarkGray };
  params.hiscores_nr_style = { Color::LightGray, Color::Black, Color::White };
  params.hiscores_score_style = { Color::LightGray, Color::Black, Color::White };
  params.hiscores_name_style = { Color::LightGray, Color::Black, Color::White };
  params.hiscores_info_style = { Color::DarkGray, Color::Black };
  params.game_over_line_0_style = { Color::Black, Color::White };
  params.game_over_line_1_style = { Color::Black, Color::LightGray };
  params.game_over_line_2_style = { Color::Black, Color::DarkGray };
  params.game_over_line_3_style = { Color::Black, Color::LightGray };
  params.game_over_line_4_style = { Color::Black, Color::White };
  
  if (argc >= 3 && strcmp(argv[2], "--log_mode") == 0)
  {
    if (strcmp(argv[3], "record") == 0)
      params.log_mode = LogMode::Record;
    else if (strcmp(argv[3], "replay") == 0)
      params.log_mode = LogMode::Replay;
    params.xcode_log_filepath = "../../../../../../../../Documents/xcode/Asciiroids/Asciiroids/";
  }
  
  Game game(argc, argv, params);

  if (argc >= 2 && strcmp(argv[1], "--help") == 0)
  {
    std::cout << "asciiroids (\"--help\" | [(<frame-delay-us> | '-') [--log_mode (record | replay)]])" << std::endl;
    std::cout << "  default values:" << std::endl;
    std::cout << "    <frame-delay-us>    : " << game.get_sim_delay_us() << std::endl;
    return EXIT_SUCCESS;
  }

  game.init();
  game.generate_data();
  game.run();

  return EXIT_SUCCESS;
}
