//
//  asciiroid_belt.cpp
//  Asciiroid_Belt
//
//  Created by Rasmus Anthin on 2025-07-05.
//

#include "Keyboard.h"
#include "TitleScreen.h"
#include "InstructionsScreen.h"

#include <Core/Timer.h>
#include <Termin8or/sys/GameEngine.h>
#include <Termin8or/sprite/SpriteHandler.h>
#include <Termin8or/title/ASCII_Fonts.h>
#include <Termin8or/physics/dynamics/RigidBody.h>
#include <Termin8or/physics/dynamics/DynamicsSystem.h>
#include <Termin8or/physics/dynamics/CollisionHandler.h>
#include <8Beat/AudioSourceHandler.h>
#include <8Beat/ChipTuneEngine.h>
#include <8Beat/WaveformGeneration.h>
#include <8Beat/SFX.h>

using RC = t8::RC;
using Color16 = t8::Color16;
using CharT = char; // char32_t;
template<int NR, int NC>
using ScreenHandler = t8::ScreenHandler<NR, NC, CharT>;
using BitmapSprite = t8x::BitmapSprite;
using VectorSprite = t8x::VectorSprite;
using RigidBody = t8x::RigidBody;

//#define DESIGN_SFX

static const float c_default_vol = 0.5f;

// ////////////////////////////
// [x] Explosion sprites.
// [x] Spaceship collision logic (explosion + reappearance, etc).
// [x] Score counting.
// [x] Shots should split larger asteroids into two smaller ones which travel faster than the original.
// [x] Hyperspace.
// [x] Large UFO.
// [x] Small UFO.
// [x] UFOs shoot at spaceship.
// [x] Spaceship can shoot at UFOs.
// [x] SFX.
// [x] Music.
// ////////////////////////////

class Game : public t8x::GameEngine<40, 100, CharT>
{
#ifdef DESIGN_SFX
  std::vector<float> vp_design;
  int channel = 0;
#endif

public:
  Game(int argc, char** argv, const t8x::GameEngineParams& params,
       bool use_audio, bool use_3d_audio,
       float music_volume, float sfx_volume)
    : GameEngine(argv[0], params)
    , audio(use_audio)
    , volume_music(music_volume)
    , volume_sfx(sfx_volume)
    , enable_audio(use_audio)
    , enable_3d_audio(use_3d_audio)
  {
  //#ifndef _WIN32
    GameEngine::set_real_fps(15);
    GameEngine::set_sim_delay_us(50'000);
    GameEngine::set_anim_rate(0, 4); // Explosion
    GameEngine::set_anim_rate(1, 3); // Asteroid sprites.
    GameEngine::set_anim_rate(2, 5); // UFO AI
  //#endif
  
    shot_freq_timer.force_start(0.f);
    
    for (int a_idx = 1; a_idx < argc; ++a_idx)
      if (std::strcmp(argv[a_idx], "--framed_splash_screen") == 0)
        framed_splash = true;
  }
  
  ~Game()
  {
    if (enable_audio)
    {
      audio.remove_source(src_fx_shot);
      audio.remove_source(src_fx_explosion);
      audio.remove_source(src_fx_ufo_shot);
      audio.remove_source(src_fx_ufo_large_propulsion);
      audio.remove_source(src_fx_ufo_small_propulsion);
    }
  }

  virtual void generate_data() override
  {
#ifndef DESIGN_SFX
    try
    {
      std::string tune_path = get_exe_folder();
    
      if (enable_audio && chip_tune.load_tune(folder::join_path({ tune_path, "music.ct" })))
      {
          chip_tune.set_volume_slider(volume_music, min_dB, nl_taper);
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
    
    if (enable_audio)
    {
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
      };
      static std::vector<float> vp_ufo_shot
      {
        0.32f,
        0.43f,
        10.f,
        0.f,
        -0.11f,
        0.5f,
        -0.25f,
        0.4f,
        0.3f,
        0.29f,
        0.7f,
        0.09f,
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
      
      float sfx_gain_factor = (enable_3d_audio ? 10.f : 2.f);

      auto wd_shot = beat::SFX::generate(beat::SFXType::LASER, vp_shot);
      src_fx_shot = audio.create_source_from_waveform(wd_shot);
      src_fx_shot->set_volume_slider(volume_sfx, min_dB, nl_taper);
      src_fx_shot->set_gain(gain_shot * sfx_gain_factor);
      src_fx_shot->enable_3d_audio(enable_3d_audio);
      if (enable_3d_audio)
      {
        src_fx_shot->set_speed_of_sound(300.f);
        src_fx_shot->set_directivity_alpha(0.8f);
        src_fx_shot->set_directivity_sharpness(1.f);
        src_fx_shot->set_directivity_type(0);
        src_fx_shot->set_rear_attenuation(0.5f);
        src_fx_shot->set_attenuation_min_distance(0.5f);
        src_fx_shot->set_attenuation_constant_falloff(1.f);
        src_fx_shot->set_attenuation_linear_falloff(0.004f);
        src_fx_shot->set_attenuation_quadratic_falloff(0.0002f);
      }
      auto wd_explosion = beat::SFX::generate(beat::SFXType::EXPLOSION, vp_explosion);
      src_fx_explosion = audio.create_source_from_waveform(wd_explosion);
      src_fx_explosion->set_volume_slider(volume_sfx, min_dB, nl_taper);
      src_fx_explosion->set_gain(gain_explosion * sfx_gain_factor);
      src_fx_explosion->enable_3d_audio(enable_3d_audio);
      if (enable_3d_audio)
      {
        src_fx_explosion->set_speed_of_sound(300.f);
        src_fx_explosion->set_directivity_alpha(0.f);
        src_fx_explosion->set_directivity_sharpness(1.f);
        src_fx_explosion->set_directivity_type(0);
        src_fx_explosion->set_rear_attenuation(1.f);
        src_fx_explosion->set_attenuation_min_distance(0.5f);
        src_fx_explosion->set_attenuation_constant_falloff(1.f);
        src_fx_explosion->set_attenuation_linear_falloff(0.004f);
        src_fx_explosion->set_attenuation_quadratic_falloff(0.0002f);
      }
      auto wd_ufo_shot = beat::SFX::generate(beat::SFXType::LASER, vp_ufo_shot);
      src_fx_ufo_shot = audio.create_source_from_waveform(wd_ufo_shot);
      src_fx_ufo_shot->set_volume_slider(volume_sfx, min_dB, nl_taper);
      src_fx_ufo_shot->set_gain(gain_ufo_shot * sfx_gain_factor);
      src_fx_ufo_shot->enable_3d_audio(enable_3d_audio);
      if (enable_3d_audio)
      {
        src_fx_ufo_shot->set_speed_of_sound(300.f);
        src_fx_ufo_shot->set_directivity_alpha(0.8f);
        src_fx_ufo_shot->set_directivity_sharpness(1.f);
        src_fx_ufo_shot->set_directivity_type(0);
        src_fx_ufo_shot->set_rear_attenuation(0.5f);
        src_fx_ufo_shot->set_attenuation_min_distance(0.5f);
        src_fx_ufo_shot->set_attenuation_constant_falloff(1.f);
        src_fx_ufo_shot->set_attenuation_linear_falloff(0.004f);
        src_fx_ufo_shot->set_attenuation_quadratic_falloff(0.0002f);
      }
      beat::WaveformGenerationParams params;
      params.vibrato_depth = 0.1f;      // 20% amplitude vibrato
      params.vibrato_freq = 6.f;       // 6 Hz wobble
      params.freq_vibrato_depth = 0.3f;
      params.freq_vibrato_freq = 6.f;
      params.freq_vibrato_phase = math::c_pi*0.5f;
      params.duty_cycle = 0.5f;         // standard triangle
      auto wd_prop = wave_gen.generate_waveform(beat::WaveformType::TRIANGLE, 10.f, 1318.f, params, 44100, false);
      src_fx_ufo_large_propulsion = audio.create_source_from_waveform(wd_prop);
      src_fx_ufo_small_propulsion = audio.create_source_from_waveform(wd_prop);
      src_fx_ufo_large_propulsion->set_volume_slider(volume_sfx, min_dB, nl_taper);
      src_fx_ufo_large_propulsion->set_gain(gain_ufo_propulsion * sfx_gain_factor);
      src_fx_ufo_small_propulsion->set_volume_slider(volume_sfx, min_dB, nl_taper);
      src_fx_ufo_small_propulsion->set_gain(gain_ufo_propulsion * sfx_gain_factor);
      src_fx_ufo_large_propulsion->enable_3d_audio(enable_3d_audio);
      src_fx_ufo_small_propulsion->enable_3d_audio(enable_3d_audio);
      if (enable_3d_audio)
      {
        src_fx_ufo_large_propulsion->set_speed_of_sound(300.f);
        src_fx_ufo_large_propulsion->set_directivity_alpha(0.8f);
        src_fx_ufo_large_propulsion->set_directivity_sharpness(1.f);
        src_fx_ufo_large_propulsion->set_directivity_type(0);
        src_fx_ufo_large_propulsion->set_rear_attenuation(0.5f);
        src_fx_ufo_large_propulsion->set_attenuation_min_distance(0.5f);
        src_fx_ufo_large_propulsion->set_attenuation_constant_falloff(1.f);
        src_fx_ufo_large_propulsion->set_attenuation_linear_falloff(0.004f);
        src_fx_ufo_large_propulsion->set_attenuation_quadratic_falloff(0.0002f);
        src_fx_ufo_small_propulsion->set_speed_of_sound(300.f);
        src_fx_ufo_small_propulsion->set_directivity_alpha(0.8f);
        src_fx_ufo_small_propulsion->set_directivity_sharpness(1.f);
        src_fx_ufo_small_propulsion->set_directivity_type(0);
        src_fx_ufo_small_propulsion->set_rear_attenuation(0.5f);
        src_fx_ufo_small_propulsion->set_attenuation_min_distance(0.5f);
        src_fx_ufo_small_propulsion->set_attenuation_constant_falloff(1.f);
        src_fx_ufo_small_propulsion->set_attenuation_linear_falloff(0.004f);
        src_fx_ufo_small_propulsion->set_attenuation_quadratic_falloff(0.0002f);
      }

      if (enable_3d_audio)
      {
        la::Mtx4 trf_l;
        // World coordsys assumed to be:
        //   Z : up towards viewer, straight out of the screen.
        //   X : towards the right edge of the screen.
        //   Y : up towards the upper edge of the screen.
        //   origin at the center of the screen.
        trf_l.set_column_vec(la::X, { -1.f, 0.f, 0.f }); // towards the left along the screen.
        trf_l.set_column_vec(la::Y, { 0.f, 1.f, 0.f }); // upwards along the screen.
        trf_l.set_column_vec(la::Z, { 0.f, 0.f, -1.f }); // towards the "inside" of the screen.
        trf_l.set_column_vec(la::W, { sh.num_cols()*0.5f, sh.num_rows()*0.5f, 0.f }); // Source world position encoded here.
        la::Vec3 pos_l_L_l { -0.12f, 0.05f, -0.05f }; // Channel Left emitter local position encoded here.
        la::Vec3 pos_l_R_l { +0.12f, 0.05f, -0.05f }; // Channel Left emitter local position encoded here.
        la::Vec3 vel_w_l = la::Vec3_Zero; // Lisitener world velocity encoded here.
        //beat::la::Vec3 ang_vel_w_l = beat::la::Vec3_Zero;
        audio.set_listener_3d_state_channel(0, trf_l.get_rot_matrix().to_arr(), trf_l.transform_pos(pos_l_L_l).to_arr(), vel_w_l.to_arr());
        audio.set_listener_3d_state_channel(1, trf_l.get_rot_matrix().to_arr(), trf_l.transform_pos(pos_l_R_l).to_arr(), vel_w_l.to_arr());
      }
    }
    
    std::string font_data_path = folder::join_path({ get_exe_folder(), "fonts" });
    std::cout << font_data_path << std::endl;
    
    auto& cs0 = color_schemes.emplace_back();
    cs0.internal.fg_color = Color16::White;
    cs0.internal.bg_color = Color16::Black;
    cs0.dot_internal.fg_color = Color16::White;
    cs0.dot_internal.bg_color = Color16::DarkGray;
    cs0.dot_side_h.fg_color = Color16::White;
    cs0.dot_side_h.bg_color = Color16::DarkGray;
    auto& cs1 = color_schemes.emplace_back();
    cs1.internal.fg_color = Color16::White;
    cs1.internal.bg_color = Color16::Black;
    
    font_data = t8x::load_font_data(font_data_path);
    
    sprite_spaceship = sprh.create_vector_sprite("spaceship");
    sprite_spaceship->layer_id = 2;
    sprite_spaceship->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_spaceship->add_line_segment(0, { 1, 1 }, { -1, 0 }, 'o', { Color16::White, Color16::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(0, { -1, 0 }, { 1, -1 }, 'o', { Color16::White, Color16::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(0, { 0.7f, -1.f }, { 0.7f, 1.f }, '.', { Color16::White, Color16::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(0, { 1, -1 }, { 0.7f, -1.f }, '.', { Color16::White, Color16::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(0, { 1, 1 }, { 0.7f, 1.f }, '.', { Color16::White, Color16::Transparent2 }, 1);
    
    sprite_spaceship->add_line_segment(1, { 1.71f, 0 }, { 1.71f, 0 }, '*', { Color16::White, Color16::Transparent2 }, 2);
    sprite_spaceship->add_line_segment(1, { 1, 1 }, { -1, 0 }, 'o', { Color16::White, Color16::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(1, { -1, 0 }, { 1, -1 }, 'o', { Color16::White, Color16::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(1, { 0.7f, -1.f }, { 0.7f, 1.f }, '.', { Color16::White, Color16::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(1, { 1, -1 }, { 0.7f, -1.f }, '.', { Color16::White, Color16::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(1, { 1, 1 }, { 0.7f, 1.f }, '.', { Color16::White, Color16::Transparent2 }, 1);
    sprite_spaceship->func_calc_anim_frame = [&](int sim_frame) { return spaceship_fwd_force > 0.f ? sim_frame % 2 : 0; };
    sprite_spaceship->set_rotation(0.f);
    sprite_spaceship->set_aspect_ratio(spaceship_ar);
    for (int frame_id = 0; frame_id < 2; ++frame_id)
    {
      sprite_spaceship->finalize_topology(frame_id);
      auto* frame = sprite_spaceship->get_curr_local_frame(frame_id);
      frame->fill_closed_polylines = false;
      frame->fill_glyph = '#';
      frame->fill_style = { Color16::LightGray, Color16::DarkGray };
    }
    rb_spaceship = dyn_sys.add_rigid_body(sprite_spaceship, 4.f,
      std::nullopt, {}, {},
      spaceship_rot_vel, 0.f,
      0.f, 0.f,
      crit_vel_r, crit_vel_c);
    rb_spaceship->set_orig_dir({ -1.f, 0.f });
    
    Color16 asteroid_bg_color = use_transparent_asteroids ? Color16::Transparent2 : Color16::Black;
    
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
    sprite_asteroid_0_big->fill_sprite_fg_colors(0, Color16::White);
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
    sprite_asteroid_0_small->fill_sprite_fg_colors(0, Color16::White);
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
    sprite_asteroid_0_tiny->fill_sprite_fg_colors(0, Color16::White);
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
    sprite_asteroid_1_big->fill_sprite_fg_colors(0, Color16::White);
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
    sprite_asteroid_1_small->fill_sprite_fg_colors(0, Color16::White);
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
    sprite_asteroid_1_tiny->fill_sprite_fg_colors(0, Color16::White);
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
    sprite_asteroid_2_big->fill_sprite_fg_colors(0, Color16::White);
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
    sprite_asteroid_2_small->fill_sprite_fg_colors(0, Color16::White);
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
    sprite_asteroid_2_tiny->fill_sprite_fg_colors(0, Color16::White);
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
    sprite_explosion->fill_sprite_fg_colors(0, Color16::White);
    sprite_explosion->fill_sprite_bg_colors(0, Color16::Transparent2);
    sprite_explosion->create_frame(1);
    sprite_explosion->set_sprite_chars_from_strings(1,
        "         ",
        "    ..   ",
        "   . ..  ",
        "    .    ",
        "         "
      );
    sprite_explosion->fill_sprite_fg_colors(1, Color16::White);
    sprite_explosion->fill_sprite_bg_colors(1, Color16::Transparent2);
    sprite_explosion->create_frame(2);
    sprite_explosion->set_sprite_chars_from_strings(2,
        "    . .  ",
        "  .  . . ",
        " . .  .  ",
        "  .    . ",
        "    .    "
      );
    sprite_explosion->fill_sprite_fg_colors(2, Color16::White);
    sprite_explosion->fill_sprite_bg_colors(2, Color16::Transparent2);
    sprite_explosion->create_frame(3);
    sprite_explosion->set_sprite_chars_from_strings(3,
        " . .. .  ",
        "  .  . . ",
        " ..    . ",
        "  .   .  ",
        " .  .   ."
      );
    sprite_explosion->fill_sprite_fg_colors(3, Color16::White);
    sprite_explosion->fill_sprite_bg_colors(3, Color16::Transparent2);
    sprite_explosion->create_frame(4);
    sprite_explosion->set_sprite_chars_from_strings(4,
        ".   .  . ",
        " .     . ",
        ".       .",
        "         ",
        " .  .   ."
      );
    sprite_explosion->fill_sprite_fg_colors(4, Color16::White);
    sprite_explosion->fill_sprite_bg_colors(4, Color16::Transparent2);
    sprite_explosion->create_frame(5);
    sprite_explosion->set_sprite_chars_from_strings(5,
        "         ",
        "         ",
        ".        ",
        "         ",
        "     .   "
      );
    sprite_explosion->fill_sprite_fg_colors(5, Color16::White);
    sprite_explosion->fill_sprite_bg_colors(5, Color16::Transparent2);
    sprite_explosion->create_frame(6);
    sprite_explosion->set_sprite_chars_from_strings(6,
        "         ",
        "         ",
        "         ",
        "         ",
        "         "
      );
    sprite_explosion->fill_sprite_fg_colors(6, Color16::White);
    sprite_explosion->fill_sprite_bg_colors(6, Color16::Transparent2);
    
    sprite_ufo_large = sprh.create_bitmap_sprite("ufo large");
    sprite_ufo_large->enabled = false;
    sprite_ufo_large->layer_id = 1;
    sprite_ufo_large->pos = { 2, sh.num_cols() - 2 };
    sprite_ufo_large->init(2, 5);
    sprite_ufo_large->create_frame(0);
    sprite_ufo_large->fill_sprite_fg_colors(0, Color16::White);
    sprite_ufo_large->fill_sprite_bg_colors(0, Color16::Transparent2);
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
    sprite_ufo_small->fill_sprite_fg_colors(0, Color16::White);
    sprite_ufo_small->fill_sprite_bg_colors(0, Color16::Transparent2);
    sprite_ufo_small->fill_sprite_materials(0, 1);
    sprite_ufo_small->set_sprite_chars_from_strings(0,
      "<^>"
    );
    
    rb_ufo_large = dyn_sys.add_rigid_body(sprite_ufo_large, 4.f);
    rb_ufo_small = dyn_sys.add_rigid_body(sprite_ufo_small, 2.f);
    
    if (!ufo_can_collide_with_asteroids)
      coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "ufo", true);
    coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ufo", "ufo", true);
  }
  
private:

  void set_sound_channel_state(beat::AudioSource* src, const Vec2& pos, const Vec2& dir, const Vec2& vel)
  {
    using namespace applaudio;
    if (!enable_3d_audio)
      return;
    la::Mtx4 trf_s;
    la::Vec3 w;
    la::Vec3 vel_3d;
    auto z = la::Vec3 { dir.c, -dir.r, 0.f }; // +Z is forward by default in applaudio, but this is a derived property from z.
    z = la::normalize(z);
    auto y = la::Vec3 { 0.f, 0.f, 1.f };
    auto x = la::normalize(la::cross(y, z)); // -X is right by default in applaudio, but this is a derived property from x.
    w = la::Vec3 { pos.c, static_cast<float>(sh.num_rows()) - pos.r, -5.f };
    trf_s.set_column_vec(la::Z, z);
    trf_s.set_column_vec(la::Y, y);
    trf_s.set_column_vec(la::X, x);
    trf_s.set_column_vec(la::W, w);
    vel_3d = { vel.c, -vel.r, 0.f };
    
    src->set_3d_state_channel(0, trf_s.get_rot_matrix().to_arr(), w.to_arr(), (vel_3d * vel_factor_3d).to_arr());
  }

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
      auto nr_f = static_cast<float>(sh.num_rows());
      auto nc_f = static_cast<float>(sh.num_cols());
      Vec2 pos;
      for (;;)
      {
        pos = Vec2 { rnd::rand_float(0.f, nr_f), rnd::rand_float(0.f, nc_f) };
        if (math::distance_squared_ar(pos, rb_spaceship->get_curr_cm(), 2.f) > c_min_ship_asteroid_dist_sq)
          break;
      }
      std::string sprite_src_name = "asteroid " + std::to_string(rnd::rand_int(0, 2)) + " big";
      asteroid.sprite = static_cast<BitmapSprite*>(sprh.clone_sprite("asteroid big id:" + std::to_string(global_asteroid_id++), sprite_src_name));
      //std::cout << asteroid.sprite->get_name() << " : " << sprite_src_name << std::endl;
      asteroid.sprite->enabled = true;
      asteroid.rb = dyn_sys.add_rigid_body(asteroid.sprite, 20.f, // mass
        pos, // pos
        Vec2 { rnd::randn(0.f, 3.f), rnd::randn(0.f, 3.f) } // vel
      );
    }
    coll_handler.rebuild_BVH(sh.num_rows(), sh.num_cols(), &dyn_sys);
    coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "ast", true);
  }
  
  template<int NR, int NC>
  void draw_hud(ScreenHandler<NR, NC>& sh)
  {
    sh.write_buffer(str::adjust_str(std::to_string(GameEngine::ref_score()), str::Adjustment::Right, 6), 1, 8, Color16::White);
  
    for (int life_idx = 0; life_idx < std::min(num_lives, 10); ++life_idx)
    {
      sh.write_buffer("A", 2, 10 + life_idx, Color16::White);
    }
  }
  
  // ///////

  virtual void update() override
  {
#ifdef DESIGN_SFX
    if (enable_audio)
    {
      auto key_held = t8::get_char_key(kpdp.held);
      auto special_key = t8::get_special_key(kpdp.held);
      for (int i = 0; i < stlutils::sizeI(vp_design); ++i)
      {
        int j_max = math::linmap(vp_design[i], -2.f, 3.f, 0, 50);
        for (int j = 0; j < j_max; ++j)
          sh.write_buffer("#", i+2, j+15, channel == i ? Color16::Yellow : Color16::White);
        sh.write_buffer(std::to_string(vp_design[i]), i+2, 5, Color16::Cyan);
        sh.write_buffer(std::to_string(i) + '.', i+2, 1, Color16::Blue);
      }
      
      auto key = t8::get_special_key(kpdp.transient);
      switch (key)
      {
        case t8::SpecialKey::Up:
          channel--;
          if (channel < 0)
            channel = 16;
          break;
        case t8::SpecialKey::Down:
          channel++;
          if (channel > 16)
            channel = 0;
          break;
        case t8::SpecialKey::Left:
          vp_design[channel] -= 0.01f;
          if (vp_design[channel] < -2.f)
            vp_design[channel] = -2.f;
          break;
        case t8::SpecialKey::Right:
          vp_design[channel] += 0.01f;
          if (vp_design[channel] > 3.f)
            vp_design[channel] = 3.f;
          break;
        default:
          break;
      }
      
      if (t8::get_char_key(kpdp.transient) == ' ')
      {
        auto wd_shot = beat::SFX::generate(beat::SFXType::LASER, vp_design);
        src_fx_shot->update_buffer(wd_shot);
      
        src_fx_shot->play();
      }
    }
#else
  
    // Game logic.
    //int frame = GameEngine::get_frame_count();
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
      
    auto f_fire_shot = [&](ShotID id, const Vec2& pos, const Vec2& dir, const Vec2& ship_vel)
    {
      if (shot_freq_timer.wait_then_reset(t))
      {
        Shot shot;
        shot.dir = dir;
        shot.dir = math::normalize(shot.dir);
        shot.pos = pos;
        shot.timer.start_if_stopped(t);
        shot.id = id;
        shots_vec.emplace_back(shot);
        shot_freq_timer.start_if_stopped(t);
        
        switch (id)
        {
          case ShotID::Spaceship:
            if (src_fx_shot != nullptr)
            {
              if (enable_3d_audio)
                set_sound_channel_state(src_fx_shot, pos, dir, ship_vel);
              src_fx_shot->play();
            }
            break;
          case ShotID::UFO:
            if (src_fx_ufo_shot != nullptr)
            {
              if (enable_3d_audio)
                set_sound_channel_state(src_fx_ufo_shot, pos, dir, ship_vel);
              src_fx_ufo_shot->play();
            }
            break;
        }
      }
    };
    
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
          f_fire_shot(ShotID::Spaceship,
            rb_spaceship->get_curr_cm() + spaceship_dir * 1.f,
            { spaceship_dir.r / spaceship_ar, spaceship_dir.c },
            rb_spaceship->get_curr_lin_vel());
          break;
        case Key::Hyperspace:
          if (hyperspace_jump_timer.start_if_stopped(t))
          {
            sprite_spaceship->enabled = false;
            coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "spa", true);
            coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ufo", "spa", true);
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
      return !shot.timer.is_ticking(t) || shot.hit;
    });
    for (auto& shot : shots_vec)
      shot.pos += shot.dir * shot_speed * dt;
    
    // Toroidal geometry update
    auto cm = t8::to_RC_round(rb_spaceship->get_curr_cm());
    if (toroidal_wrap(sh, cm, 0, 1))
      rb_spaceship->set_curr_cm(to_Vec2(cm));
    for (auto& shot : shots_vec)
    {
      auto pos = t8::to_RC_round(shot.pos);
      if (toroidal_wrap(sh, pos, 0, 0))
        shot.pos = to_Vec2(pos);
    }
    for (auto& asteroid : asteroids_vec)
    {
      auto a_cm = t8::to_RC_round(asteroid.rb->get_curr_cm());
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
      if (src_fx_explosion != nullptr)
      {
        set_sound_channel_state(src_fx_explosion, to_Vec2(pos), { 0.f, -1.f }, { 0.f, 0.f });
        src_fx_explosion->play();
      }
      return expl_raw_ptr;
    };
    auto isect_data = coll_handler.get_isect_world_positions();
    for (const auto& id : isect_data)
    {
      if (!stlutils::contains_if(explosions_vec, [&id](const auto& expl) { return (id.node_A == expl->isect_data.node_A && id.node_B == expl->isect_data.node_B) || (id.node_A == expl->isect_data.node_B && id.node_B == expl->isect_data.node_A); }))
      {
        auto* explosion = f_generate_explosion(t8::to_RC_round(id.world_pos));
        explosion->isect_data = id;
      }
    }
    
    // Shots: UFO -> spaceship, spaceship -> UFO.
    for (auto& shot : shots_vec)
    {
      auto shot_rc = t8::to_RC_round(shot.pos);
      if (sprite_ufo_large->enabled && shot.id == ShotID::Spaceship)
      {
        if (sprite_ufo_large->calc_curr_AABB(0).contains(shot_rc))
        {
          f_generate_explosion(shot_rc);
          ufo_active_timer.reset();
          shot.hit = true;
          GameEngine::ref_score() += 200;
        }
      }
      else if (sprite_ufo_small->enabled && shot.id == ShotID::Spaceship)
      {
        if (sprite_ufo_small->calc_curr_AABB(0).contains(shot_rc))
        {
          f_generate_explosion(shot_rc);
          ufo_active_timer.reset();
          shot.hit = true;
          GameEngine::ref_score() += 1000;
        }
      }
      else if (sprite_spaceship->enabled && shot.id == ShotID::UFO)
      {
        if (sprite_spaceship->calc_curr_AABB(0).contains(shot_rc))
        {
          f_generate_explosion(shot_rc);
          sprite_spaceship->enabled = false;
          spaceship_reappearance_timer.force_start(t);
          coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "spa", true);
          coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ufo", "spa", true);
          num_lives--;
        }
      }
    }

    // Split asteroids when shot upon.
    new_asteroids_vec.clear();
    bool asteroids_removed = false;
    for (auto& asteroid : asteroids_vec)
    {
      auto aabb_asteroid = asteroid.sprite->calc_curr_AABB(0);
      RC hit_rc;
      for (auto& shot : shots_vec)
      {
        auto shot_rc = t8::to_RC_round(shot.pos);
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
        asteroids_removed = true;
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
    if (asteroids_removed)
    {
      coll_handler.rebuild_BVH(sh.num_rows(), sh.num_cols(), &dyn_sys);
      coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "ast", true);
    }
    
    // If spaceship collided with something then lose a life and make the ship disappear and reappear in a safe zone.
    auto id_it = stlutils::find_if(isect_data, [&](const auto& id) { return id.node_A->rigid_body == rb_spaceship || id.node_B->rigid_body == rb_spaceship; });
    if (id_it != isect_data.end() && spaceship_reappearance_timer.start_if_stopped(t))
    {
      sprite_spaceship->enabled = false;
      coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "spa", true);
      coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ufo", "spa", true);
      num_lives--;
    }
    if (spaceship_reappearance_timer.finished(t))
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
        coll_handler.reinclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ufo", "spa");
      }
    }
    
    // Hyperspace jump destination.
    if (hyperspace_jump_timer.finished(t))
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
          coll_handler.reinclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ufo", "spa");
          break;
        }
        if (iter++ > 5) // Making sure we don't spend too much time here and stall the main thread.
          break;
      }
    }
    
    // UFOs
    if (ufo_active_timer.is_ticking(t))
    {
      RigidBody* ufo_rb = nullptr;
      int ufo_size = -1; // 0 : large, 1 : small.
      if (sprite_ufo_large->enabled)
      {
        ufo_rb = rb_ufo_large;
        ufo_size = 0;
      }
      else if (sprite_ufo_small->enabled)
      {
        ufo_rb = rb_ufo_small;
        ufo_size = 1;
      }
      else
        std::cerr << "ERROR : An UFO was expected to be active but none was found!\n";
      
      Vec2 pos = ufo_rb->get_curr_cm();
      ufo_rb->set_curr_lin_vel({ 0.f, 0.f });
      
      if (ufo_h_dir == +1)
        pos.c += ufo_delta_pos;
      else if (ufo_h_dir == -1)
        pos.c -= ufo_delta_pos;
        
      if (ufo_v_dir == +1)
        pos.r -= ufo_delta_pos/2.5f;
      else if (ufo_v_dir == -1)
        pos.r += ufo_delta_pos/2.5f;
        
      auto rc = t8::to_RC_round(pos);
      if (toroidal_wrap(sh, rc, 0, 0))
        pos = to_Vec2(rc);
        
      ufo_rb->set_curr_cm(pos);
        
      if (!ufo_v_move_timer.is_ticking(t) && rnd::one_in(50))
      {
        ufo_v_dir = rnd::rand_int(-1, +1);
        ufo_v_move_timer.start_if_stopped(t);
        if (ufo_v_dir == 0)
          ufo_v_move_timer.set_delay(rnd::randn_clamp(3.f, 1.f, 0.5f, 5.f));
        else
          ufo_v_move_timer.set_delay(rnd::randn_clamp(1.5f, 1.f, 0.5f, 5.f));
      }
      
      if (enable_3d_audio)
      {
        auto ufo_dir = rb_ufo_large->get_curr_dir();
        if (src_fx_ufo_large_propulsion->is_playing())
          set_sound_channel_state(src_fx_ufo_large_propulsion, pos, ufo_dir, { 0.f, 0.f });
        else if (src_fx_ufo_small_propulsion->is_playing())
          set_sound_channel_state(src_fx_ufo_small_propulsion, pos, ufo_dir, { 0.f, 0.f });
      }
      
      if (!ufo_shot_interval_timer.is_ticking(t))
      {
        Vec2 dir = math::normalize(rb_spaceship->get_curr_cm() - pos);
        auto sigma = ufo_size == 0 ? 0.7f : 0.35f;
        dir.r += rnd::randn(0.f, sigma)/spaceship_ar;
        dir.c += rnd::randn(0.f, sigma);
        dir = math::normalize(dir);
        f_fire_shot(ShotID::UFO,
            pos,
            dir,
            ufo_rb->get_curr_lin_vel());
        ufo_shot_interval_timer.force_start(t);
      }
      
      // UFO collision handling.
      auto id_it = stlutils::find_if(isect_data, [&](const auto& id) { return id.node_A->rigid_body == rb_ufo_large || id.node_B->rigid_body == rb_ufo_large || id.node_A->rigid_body == rb_ufo_small || id.node_B->rigid_body == rb_ufo_small; });
      if (id_it != isect_data.end())
        ufo_active_timer.reset();
    }
    else
    {
      // Finished.
      if (ufo_trig.once())
      {
        ufo_active_timer.reset();
        sprite_ufo_large->enabled = false;
        sprite_ufo_small->enabled = false;
        if (ufo_can_collide_with_asteroids)
          coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "ufo", true);
        coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "spa", "ufo", true);
        if (src_fx_ufo_large_propulsion != nullptr)
          src_fx_ufo_large_propulsion->stop();
        if (src_fx_ufo_small_propulsion != nullptr)
          src_fx_ufo_small_propulsion->stop();
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
          if (iters++ > 10)
            break;
        }
        return false;
      };
      Vec2 pos;
      if (rnd::one_in(600) && f_set_ufo_pos(pos, ufo_h_dir))
      {
        ufo_active_timer.start_if_stopped(t);
        ufo_trig.reset();
        sprite_ufo_large->enabled = true;
        rb_ufo_large->set_orig_dir({ -1.f, 0.f });
        rb_ufo_large->set_curr_cm(pos);
        if (ufo_can_collide_with_asteroids)
          coll_handler.reinclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "ufo");
        coll_handler.reinclude_all_rigid_bodies_of_prefixes(&dyn_sys, "spa", "ufo");
        if (src_fx_ufo_large_propulsion != nullptr)
          src_fx_ufo_large_propulsion->play();
      }
      else if (rnd::one_in(1200) && f_set_ufo_pos(pos, ufo_h_dir))
      {
        ufo_active_timer.start_if_stopped(t);
        ufo_trig.reset();
        sprite_ufo_small->enabled = true;
        rb_ufo_small->set_orig_dir({ -1.f, 0.f });
        rb_ufo_small->set_curr_cm(pos);
        if (ufo_can_collide_with_asteroids)
          coll_handler.reinclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "ufo");
        coll_handler.reinclude_all_rigid_bodies_of_prefixes(&dyn_sys, "spa", "ufo");
        if (src_fx_ufo_small_propulsion != nullptr)
          src_fx_ufo_small_propulsion->play();
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
    if (asteroids_vec.empty() && !sprite_ufo_large->enabled && !sprite_ufo_small->enabled && level_timer.start_if_stopped(t))
    {
      cleanup_asteroids();
      if (enable_audio)
        chip_tune.stop_tune_async();
    }
    else if (level_timer.wait_then_reset(t))
    {
      level++;
      generate_big_asteroids(level*2);
      if (enable_audio)
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
    
    draw_frame(sh, Color16::LightGray);
    
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
      sh.write_buffer(".", math::roundI(shot.pos.r), math::roundI(shot.pos.c), Color16::White);
#endif
  }
  
  virtual void on_quit() override
  {
    if (enable_audio)
      chip_tune.stop_tune_async();
  }
  
  virtual void draw_title() override
  {
    ::draw_title(sh, font_data, color_schemes[0], get_exe_folder(), framed_splash);
  }
  
  virtual void draw_instructions() override
  {
    ::draw_instructions(sh, font_data, color_schemes[1]);
  }
  
  virtual void on_exit_instructions() override
  {
    //if (enable_audio)
    //  chip_tune.stop_tune_async();
  }
  
  virtual void on_enter_paused() override
  {
    if (enable_audio)
      chip_tune.pause();
  }
  
  virtual void on_exit_paused() override
  {
    if (enable_audio)
      chip_tune.resume();
  }
  
  virtual void on_enter_game_over() override
  {
    if (enable_audio)
      chip_tune.stop_tune_async();
  }
  
  virtual void on_enter_input_hiscore() override
  {
    if (enable_audio)
      chip_tune.stop_tune_async();
  }

  //////////////////////////////////////////////////////////////////////////
  
  beat::AudioSourceHandler audio;
  beat::WaveformGeneration wave_gen;
  beat::ChipTuneEngine chip_tune { audio, wave_gen };
  beat::AudioSource* src_fx_shot = nullptr;
  beat::AudioSource* src_fx_explosion = nullptr;
  beat::AudioSource* src_fx_ufo_shot = nullptr;
  beat::AudioSource* src_fx_ufo_large_propulsion = nullptr;
  beat::AudioSource* src_fx_ufo_small_propulsion = nullptr;
  
  std::vector<t8x::ColorScheme> color_schemes;
  t8x::FontDataColl font_data;
  
  bool framed_splash = false;
  
  t8x::SpriteHandler sprh;
  t8x::DynamicsSystem dyn_sys;
  t8x::CollisionHandler coll_handler;
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
  RigidBody* rb_spaceship = nullptr;
  
  const float spaceship_ar = 2.f;
  float spaceship_rot_vel = 0.f;
  float spaceship_fwd_force = 0.f;
  Vec2 spaceship_force { 0.f, 0.f };
  Vec2 spaceship_dir { -1.f, 0.f };
  float crit_vel_c = 30.f;
  float crit_vel_r = crit_vel_c/1.5f;
  Timer spaceship_reappearance_timer { 2.f };
  Timer hyperspace_jump_timer { 1.5f };
  
  float shot_speed = 31.f;
  Timer shot_freq_timer { 0.1f }; // Minimum time allowed between shots.
  enum class ShotID { Spaceship, UFO };
  struct Shot
  {
    Vec2 dir;
    Vec2 pos;
    bool hit = false;
    Timer timer { 2.f };
    ShotID id = ShotID::Spaceship;
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
    RigidBody* rb = nullptr;
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
    t8x::CollisionHandler::IsectData isect_data;
    BitmapSprite* sprite = nullptr;
  };
  int global_explosion_id = 0;
  std::vector<std::unique_ptr<ExplosionData>> explosions_vec;
  
  BitmapSprite* sprite_ufo_large = nullptr;
  BitmapSprite* sprite_ufo_small = nullptr;
  RigidBody* rb_ufo_large = nullptr;
  RigidBody* rb_ufo_small = nullptr;
  Timer ufo_active_timer { 8.f };
  Timer ufo_v_move_timer { 2.f }; // 2s vertical travel.
  OneShot ufo_trig;
  Vec2 ufo_shot_dir;
  // UFO Motion e.g.
  // +-------------------+
  // |     /----\        |
  // |----/      \       |
  // |            \----/ |
  // +-------------------+
  int ufo_h_dir = -1; // -1 : left, +1 : right.
  int ufo_v_dir = 0; // -1 : down, 0 : unchanged, +1 : up.
  float ufo_delta_pos = 0.4f;
  const bool ufo_can_collide_with_asteroids = false;
  Timer ufo_shot_interval_timer { 1.f };
  
  float volume_music = c_default_vol;
  float volume_sfx = c_default_vol;
  float gain_shot = 0.2f;
  float gain_ufo_shot = 0.2f;
  float gain_explosion = 1.f;
  float gain_ufo_propulsion = 0.15f;
  bool enable_audio = true;
  bool enable_3d_audio = true;
  float vel_factor_3d = 5.f;
  float min_dB = -60.f;
  float nl_taper = 0.28f;
};

//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  t8x::GameEngineParams params;
  params.screen_bg_color_default = Color16::Black;
  params.screen_bg_color_title = Color16::Black;
  params.screen_bg_color_instructions = Color16::Black;
  params.enable_title_screen = true;
  params.enable_instructions_screen = false;
  params.pause_info_style = { Color16::White, Color16::Transparent };
  params.screen_bg_color_quit_confirm = std::nullopt;
  params.quit_confirm_title_style = { Color16::LightGray, Color16::Transparent };
  params.quit_confirm_button_style = { Color16::Black, Color16::DarkGray, Color16::LightGray };
  params.quit_confirm_info_style = { Color16::DarkGray, Color16::Transparent };
  params.screen_bg_color_input_hiscore = std::nullopt;
  params.input_hiscore_title_style = { Color16::LightGray, Color16::Transparent };
  params.input_hiscore_prompt_style = { Color16::White, Color16::DarkGray, Color16::LightGray };
  params.input_hiscore_info_style = { Color16::DarkGray, Color16::Transparent };
  params.screen_bg_color_hiscores = std::nullopt;
  params.hiscores_title_style = { Color16::LightGray, Color16::DarkGray };
  params.hiscores_nr_style = { Color16::LightGray, Color16::Black, Color16::White };
  params.hiscores_score_style = { Color16::LightGray, Color16::Black, Color16::White };
  params.hiscores_name_style = { Color16::LightGray, Color16::Black, Color16::White };
  params.hiscores_info_style = { Color16::DarkGray, Color16::Black };
  params.game_over_line_0_style = { Color16::Black, Color16::White };
  params.game_over_line_1_style = { Color16::Black, Color16::LightGray };
  params.game_over_line_2_style = { Color16::Black, Color16::DarkGray };
  params.game_over_line_3_style = { Color16::Black, Color16::LightGray };
  params.game_over_line_4_style = { Color16::Black, Color16::White };
  
  bool use_audio = true;
  bool use_3d_audio = true;
  bool show_help = false;
  float music_volume = c_default_vol;
  float sfx_volume = c_default_vol;
  
  for (int i = 1; i < argc; ++i)
  {
    if (std::strcmp(argv[i],  "--suppress_tty_output") == 0)
      params.suppress_tty_output = true;
    else if (std::strcmp(argv[i], "--suppress_tty_input") == 0)
      params.suppress_tty_input = true;
    else if (i + 1 < argc && std::strcmp(argv[i], "--log_mode") == 0)
    {
      if (std::strcmp(argv[i + 1], "record") == 0)
        params.log_mode = LogMode::Record;
      else if (std::strcmp(argv[i + 1], "replay") == 0)
        params.log_mode = LogMode::Replay;
    }
    else if (std::strcmp(argv[i], "--disable_audio") == 0)
      use_audio = false;
    else if (std::strcmp(argv[i], "--disable_3d_audio") == 0)
      use_3d_audio = false;
    else if (std::strcmp(argv[i], "--music_volume") == 0)
      music_volume = static_cast<float>(std::atof(argv[i + 1]));
    else if (std::strcmp(argv[i], "--sfx_volume") == 0)
      sfx_volume = static_cast<float>(std::atof(argv[i + 1]));
    else if (std::strcmp(argv[i], "--help") == 0)
      show_help = true;
  }
  
  if (show_help)
    use_audio = false;
  
  Game game(argc, argv, params,
            use_audio, use_3d_audio,
            std::clamp(music_volume, 0.f, 1.f), std::clamp(sfx_volume, 0.f, 1.f));
  
  if (show_help)
  {
    std::cout << "asciiroid_belt --help |" << std::endl;
    std::cout << "   [--log_mode (record | replay)]" << std::endl;
    std::cout << "   [--suppress_tty_output]" << std::endl;
    std::cout << "   [--suppress_tty_input]" << std::endl;
    std::cout << "   [--set_fps <fps>]" << std::endl;
    std::cout << "   [--set_sim_delay_us <delay_us>]" << std::endl;
    std::cout << "   [--disable_audio]" << std::endl;
    std::cout << "   [--disable_3d_audio]" << std::endl;
    std::cout << "   [--music_volume <music_vol>]" << std::endl;
    std::cout << "   [--sfx_volume <sfx_vol>]" << std::endl;
    std::cout << "   [--framed_splash_screen]" << std::endl;
    std::cout << std::endl;
    std::cout << "  default values:" << std::endl;
    std::cout << "    <fps>       : " << game.get_real_fps() << std::endl;
    std::cout << "    <delay_us>  : " << game.get_sim_delay_us() << std::endl;
    std::cout << "    <music_vol> : " << c_default_vol << " (valid range: [0, 1])" <<std::endl;
    std::cout << "    <sfx_vol>   : " << c_default_vol << " (valid range: [0, 1])" <<std::endl;
    return EXIT_SUCCESS;
  }
  
  for (int i = 1; i < argc; ++i)
  {
    if (i + 1 < argc && std::strcmp(argv[i], "--set_fps") == 0)
      game.set_real_fps(static_cast<float>(std::atof(argv[i + 1])));
    else if (i + 1 < argc && std::strcmp(argv[i], "--set_sim_delay_us") == 0)
      game.set_sim_delay_us(static_cast<float>(std::atof(argv[i + 1])));
  }

  game.run();

  return EXIT_SUCCESS;
}
