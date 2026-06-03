//
//  TitleScreen.h
//  Asciiroid_Belt
//
//  Created by Rasmus Anthin on 2025-07-05.
//

#pragma once
#include <Termin8or/title/ASCII_Fonts.h>
#include <Termin8or/drawing/TextureFile.h>
#include <Termin8or/drawing/Drawing.h>
#include <Termin8or/screen/ScreenUtils.h>
#include <Core/FolderHelper.h>
//40x100

//    00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001
//    00000000001111111111222222222233333333334444444444555555555666666666667777777777888888888899999999990
//    01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
//--------------------------------------------------------------------------------------------------------+
//00//                                                                           Rasmus Anthin            |
//01//                                                                             Presents:              |
//02//                                                                                                    |
//03//                                                                                                    |
//04//                                                                                                    |
//05//                                                                                                    |
//06//                                                                                                    |
//07//                                                                                                    |
//08//                                                                                                    |
//09//                                                                                                    |
//10//                                                                                                    |
//11//                                                                                                    |
//12//                                                                                                    |
//13//                                                                                                    |
//14//                                                                                                    |
//15//                                                                                                    |
//16//                                                                                                    |
//17//                                                                                                    |
//18//                                                                                                    |
//19//                                                                                                    |
//20//                                                                                                    |
//21//                                                                                                    |
//22//                                                                                                    |
//23//                                                                                                    |
//24//                                                                                                    |
//25//                                                                                                    |
//26//                                                                                                    |
//27//                                                                                                    |
//28//                                                                                                    |
//29//                                                                                                    |
//30//                                                                                                    |
//31//                                                                                                    |
//32//                                                                                                    |
//33//                                                                                                    |
//34//                                                                                                    |
//35//                                                                                                    |
//36//                                                                                                    |
//37//                                                                                                    |
//38//                                                                                                    |
//39//                                                                                     (c) 2025       |
//40//----------------------------------------------------------------------------------------------------+
template<int NR, int NC, typename CharT>
void draw_title(t8::ScreenHandler<NR, NC, CharT>& sh, const t8x::FontDataColl& font_data, const t8x::ColorScheme& font_colors, const std::string& exe_folder, bool framed)
{
  using Color16 = t8::Color16;

  sh.write_buffer(" Rasmus Anthin ", framed ? 1 : 0, 75, Color16::Black, Color16::White);
  
  sh.write_buffer(" Presents: ", framed ? 2 : 1, 77, Color16::Black, Color16::White);
  
  sh.write_buffer(" (c) 2025 ", framed ? 38 : 39, framed ? 89 : 90, Color16::Black, Color16::White);
  
  std::string title = "Asciiroid Belt";
  t8x::Font font = t8x::Font::Avatar;
  auto text_width = calc_text_width(font_data, title, font);
  t8x::draw_text(sh, font_data, font_colors, title, 32, (NC - text_width)/2, font);
  
  sh.write_buffer("Press space-bar to continue...", 38, 33, Color16::Black, Color16::White);
  
  t8::Texture tex_splash;
  
  auto filepath_tex = folder::join_path({ exe_folder, "asciiroid_belt.tx" });
  t8::TextureFile::load(tex_splash, filepath_tex);
  
  if (framed)
    t8::draw_frame(sh, Color16::LightGray);
  
  t8x::draw_box_textured(sh,
                         -1, -1, tex_splash.size.r, tex_splash.size.c,
                         t8x::SolarDirection::Zenith,
                         tex_splash);
  
  //sh.replace_bg_color(Color16::White, t8::Rectangle { 32, 23, 5, 51 });
}
