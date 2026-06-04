//
//  TitleScreen.h
//  Asciiroid_Belt
//
//  Created by Rasmus Anthin on 2025-07-05.
//

#pragma once
#include <Termin8or/title/ASCII_Fonts.h>
#include <Termin8or/drawing/Drawing.h>
#include <Termin8or/screen/ScreenUtils.h>
#include <Core/FolderHelper.h>
#include <Core/TextIO.h>
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
  
  t8x::Font font = t8x::Font::Avatar;
  auto f_draw_title = [&](const std::string& title)
  {
    auto text_width = calc_text_width(font_data, title, font);
    t8x::draw_text(sh, font_data, font_colors, title, 32, (NC - text_width)/2, font);
  };
  std::vector<std::string> lines;
  if (TextIO::read_file(folder::join_file_path({ exe_folder, "title.txt" }), lines) && !lines.empty())
  {
    if (lines.size() == 2)
    {
      const auto& font_str = lines[1];
      auto ret = t8x::parse_font(font_str);
      if (ret.has_value())
        font = ret.value();
    }
    f_draw_title(lines[0]);
  }
  else
    f_draw_title("Asciiroid Belt");
  
  sh.write_buffer("Press space-bar to continue...", 38, 33, Color16::Black, Color16::White);
  if (framed)
    t8::draw_frame(sh, Color16::LightGray);
}
