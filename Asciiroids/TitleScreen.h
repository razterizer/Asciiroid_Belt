//
//  TitleScreen.h
//  Asciiroids
//
//  Created by Rasmus Anthin on 2025-07-05.
//

#pragma once
#include <Termin8or/ASCII_Fonts.h>

//    000000000011111111112222222222333333333344444444445555555556666666666677777777778
//    012345678901234567890123456789012345678901234567890123456789012345678901234567890
//------------------------------------------------------------------------------------+
//00//                                                       Rasmus Anthin            |
//01//            __   ((((((                  ,######         Presents:              |
//02//                ((((((((((((((      /################                     //    |
//03//             ___,(((((((((((((((((#####////####////#####          __ ..   ///   |
//04//__. ####         .,((((((((((((((###///////####//////####.    ____ . .   ////   |
//05//    #####         ...(((((((((((###////////####////////###.              ////   |
//06// __.######\         .(############################################@@@    ////   |
//07//    ##############################################################   @@@ ////   |
//08//    ##############################################################---   @ ///   |
//09//    ##############################################################       @/(    |
//10//_ _.########################(((((((((((((((((#####################""""""  ((*   |
//11//    ###((((((############(((((((((((((((((((######################______ @///   |
//12// __..((((((###########((((((((((((((((((((########################"""""@@////   |
//13//    ((((       '===(((((((((((((((((((((#*#\#|#/#*################@@@@@  ////   |
//14//                 ((((((((((((((((((((===(=-=(*)=-=)=============         ////   |
//15//               ((((((((((((((((((((      * / | \ *               ___ . . ////   |
//16//          ___ (((((((((((((((((              |                            ///   |
//17//       __.    ((((((((((((((                 |              ___  _  . ..  //    |
//18//               (((((((((     .               |                                  |
//19//                   ,     _)  )               |                                  |
//20//    ~~' `_        (,   _)      )                 ,------.                       |
//21//   '       .        `~~~~~=---'            __A__/_)(_)(_)\_____   ___ _         |
//22// _(     _)  )                             /                    \ .___           |
//23//(  `      '  )                            \____________________/  .._ __        |
//24// `_'`~~____  _  _     ____ _____    _____ ____  _  ____  ____  ____  _____      |
//25//      /  __\/ \/ \   /  _ Y__ __\  /  __//  __\/ \/ ___\/  _ \/  _ \/  __/      |
//26//      |  \/|| || |   | / \| / \    |  \  |  \/|| ||    \| / \|| | \||  \        |
//27//      |  __/| || |_/\| \_/| | |    |  /_ |  __/| |\___ || \_/|| |_/||  /_       |
//28//      \_/   \_/\____/\____/ \_/    \____\\_/   \_/\____/\____/\____/\____\      |
//29//                                                                 (c) 2022       |
//30//--------------------------------------------------------------------------------+
template<int NR, int NC>
void draw_title(ScreenHandler<NR, NC>& sh, const ASCII_Fonts::FontDataColl& font_data, const ASCII_Fonts::ColorScheme& font_colors)
{
  sh.write_buffer("Rasmus Anthin", 0, 55, Color::Black, Color::Cyan);
  sh.write_buffer("__", 1, 12, Color::DarkBlue);
  sh.write_buffer("((((((,", 1, 17, Color::DarkYellow, Color::Yellow);
  sh.write_buffer(",######", 1, 41, Color::DarkRed, Color::Red);
  sh.write_buffer("Presents:", 1, 57, Color::Black, Color::Cyan);

  sh.write_buffer("((((((((((((((", 2, 16, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("/################", 2, 36, Color::DarkRed, Color::Red);
  sh.write_buffer("//", 2, 74, Color::DarkGray, Color::LightGray);

  sh.write_buffer("___,", 3, 13, Color::DarkBlue);
  sh.write_buffer("(((((((((((((((((", 3, 17, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("#####", 3, 34, Color::DarkRed, Color::Red);
  sh.write_buffer("////", 3, 39, Color::DarkBlue);
  sh.write_buffer("####", 3, 43, Color::DarkRed, Color::Red);
  sh.write_buffer("//", 3, 47, Color::DarkBlue);
  sh.write_buffer("\\", 3, 49, Color::Cyan);
  sh.write_buffer("/", 3, 50, Color::DarkBlue);
  sh.write_buffer("#####", 3, 51, Color::DarkRed, Color::Red);
  sh.write_buffer("__ ..", 3, 66, Color::DarkBlue);
  sh.write_buffer("///", 3, 74, Color::DarkGray, Color::LightGray);

  sh.write_buffer("__.", 4, 0, Color::DarkBlue);
  sh.write_buffer("####", 4, 4, Color::DarkRed, Color::Red);
  sh.write_buffer(".,", 4, 17, Color::DarkBlue);
  sh.write_buffer("((((((((((((((", 4, 19, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("###", 4, 33, Color::DarkRed, Color::Red);
  sh.write_buffer("///", 4, 36, Color::DarkBlue);
  sh.write_buffer("\\", 4, 39, Color::Cyan);
  sh.write_buffer("///", 4, 40, Color::DarkBlue);
  sh.write_buffer("####", 4, 43, Color::DarkRed, Color::Red);
  sh.write_buffer("///", 4, 47, Color::DarkBlue);
  sh.write_buffer("\\", 4, 50, Color::Cyan);
  sh.write_buffer("//", 4, 51, Color::DarkBlue);
  sh.write_buffer("####.", 4, 53, Color::DarkRed, Color::Red);
  sh.write_buffer("____ . .", 4, 62, Color::DarkBlue);
  sh.write_buffer("////", 4, 73, Color::DarkGray, Color::LightGray);

  sh.write_buffer("#####", 5, 4, Color::DarkRed, Color::Red);
  sh.write_buffer("...", 5, 18, Color::DarkBlue);
  sh.write_buffer("(((((((((((", 5, 21, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("###", 5, 32, Color::DarkRed, Color::Red);
  sh.write_buffer("//", 5, 35, Color::DarkBlue);
  sh.write_buffer("\\", 5, 37, Color::Cyan);
  sh.write_buffer("/////", 5, 38, Color::DarkBlue);
  sh.write_buffer("####", 5, 43, Color::DarkRed, Color::Red);
  sh.write_buffer("////////", 5, 47, Color::DarkBlue);
  sh.write_buffer("###.", 5, 55, Color::DarkRed, Color::Red);
  sh.write_buffer("////", 5, 73, Color::DarkGray, Color::LightGray);

  sh.write_buffer("__.", 6, 1, Color::DarkBlue);
  sh.write_buffer("######\\", 6, 4, Color::DarkRed, Color::Red);
  sh.write_buffer(".", 6, 20, Color::DarkBlue);
  sh.write_buffer("(", 6, 21, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("############################################", 6, 22, Color::DarkRed, Color::Red);
  sh.write_buffer("@@@", 6, 66, Color::Black, Color::DarkGray);
  sh.write_buffer("////", 6, 73, Color::DarkGray, Color::LightGray);

  sh.write_buffer("##############################################################", 7, 4, Color::DarkRed, Color::Red);
  sh.write_buffer("   @@@", 7, 66, Color::Black, Color::DarkGray);
  sh.write_buffer("////", 7, 73, Color::DarkGray, Color::LightGray);

  sh.write_buffer("##############################################################", 8, 4, Color::DarkRed, Color::Red);
  sh.write_buffer("---   @", 8, 66, Color::Black, Color::DarkGray);
  sh.write_buffer("///", 8, 74, Color::DarkGray, Color::LightGray);

  sh.write_buffer("##############################################################", 9, 4, Color::DarkRed, Color::Red);
  sh.write_buffer("       @", 9, 66, Color::Black, Color::DarkGray);
  sh.write_buffer("/(", 9, 74, Color::DarkGray, Color::LightGray);

  sh.write_buffer("_ _.", 10, 0, Color::DarkBlue);
  sh.write_buffer("########################", 10, 4, Color::DarkRed, Color::Red);
  sh.write_buffer("(((((((((((((((((", 10, 28, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("#####################", 10, 45, Color::DarkRed, Color::Red);
  sh.write_buffer("\"\"\"\"\"\"  ", 10, 66, Color::Black, Color::DarkGray);
  sh.write_buffer("((*", 10, 74, Color::DarkGray, Color::LightGray);

  sh.write_buffer("###", 11, 4, Color::Red, Color::DarkRed);
  sh.write_buffer("((((((", 11, 7, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("############", 11, 13, Color::Red, Color::DarkRed);
  sh.write_buffer("(((((((((((((((((((", 11, 25, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("######################", 11, 44, Color::Red, Color::DarkRed);
  sh.write_buffer("______ @", 11, 66, Color::DarkGray, Color::Black);
  sh.write_buffer("///", 11, 74, Color::DarkGray, Color::LightGray);

  sh.write_buffer("__..", 12, 1, Color::DarkBlue);
  sh.write_buffer("((((((", 12, 5, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("###########", 12, 11, Color::Red, Color::DarkRed);
  sh.write_buffer("((((((((((((((((((((", 12, 22, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("########################", 12, 42, Color::Red, Color::DarkRed);
  sh.write_buffer("\"\"\"\"\"@@", 12, 66, Color::DarkGray, Color::Black);
  sh.write_buffer("////", 12, 73, Color::DarkGray, Color::LightGray);

  sh.write_buffer("((((", 13, 4, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("'===", 13, 15, Color::Red, Color::DarkRed);
  sh.write_buffer("(((((((((((((((((((((", 13, 19, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("#", 13, 40, Color::Red, Color::DarkRed);
  sh.write_buffer("*", 13, 41, Color::Red, Color::DarkYellow);
  sh.write_buffer("#", 13, 42, Color::Red, Color::DarkRed);
  sh.write_buffer("\\", 13, 43, Color::Yellow, Color::DarkYellow);
  sh.write_buffer("#", 13, 44, Color::Red, Color::Red);
  sh.write_buffer("|", 13, 45, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("#", 13, 46, Color::Red, Color::Red);
  sh.write_buffer("/", 13, 47, Color::Yellow, Color::DarkYellow);
  sh.write_buffer("#", 13, 48, Color::Red, Color::DarkRed);
  sh.write_buffer("*", 13, 49, Color::Red, Color::DarkYellow);
  sh.write_buffer("################", 13, 50, Color::Red, Color::DarkRed);
  sh.write_buffer("@@@@@", 13, 66, Color::Black, Color::Black);
  sh.write_buffer("////", 13, 73, Color::DarkGray, Color::LightGray);

  sh.write_buffer("((((((((((((((((((((", 14, 17, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("===", 14, 37, Color::Red, Color::DarkRed);
  sh.write_buffer("(", 14, 40, Color::DarkRed, Color::DarkYellow);
  sh.write_buffer("=", 14, 41, Color::Red, Color::DarkRed);
  sh.write_buffer("-", 14, 42, Color::Yellow, Color::DarkYellow);
  sh.write_buffer("=", 14, 43, Color::Red, Color::Red);
  sh.write_buffer("(", 14, 44, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("*", 14, 45, Color::Cyan, Color::Yellow);
  sh.write_buffer(")", 14, 46, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("=", 14, 47, Color::Red, Color::Red);
  sh.write_buffer("-", 14, 48, Color::Yellow, Color::DarkYellow);
  sh.write_buffer("=", 14, 49, Color::Red, Color::DarkRed);
  sh.write_buffer(")", 14, 50, Color::DarkRed, Color::DarkYellow);
  sh.write_buffer("=============", 14, 51, Color::Red, Color::DarkRed);
  sh.write_buffer("////", 14, 73, Color::DarkGray, Color::LightGray);

  sh.write_buffer("((((((((((((((((((((", 15, 15, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("*", 15, 41, Color::Red, Color::DarkYellow);
  sh.write_buffer("/", 15, 43, Color::Yellow, Color::DarkYellow);
  sh.write_buffer("|", 15, 45, Color::Green, Color::Yellow);
  sh.write_buffer("\\", 15, 47, Color::Yellow, Color::DarkYellow);
  sh.write_buffer("*", 15, 49, Color::Red, Color::DarkYellow);
  sh.write_buffer("___ . .", 15, 65, Color::DarkBlue);
  sh.write_buffer("////", 15, 73, Color::DarkGray, Color::LightGray);

  sh.write_buffer("___", 16, 10, Color::DarkBlue);
  sh.write_buffer("(((((((((((((((((", 16, 14, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("|", 16, 45, Color::Green, Color::Cyan);
  sh.write_buffer("///", 16, 74, Color::DarkGray, Color::LightGray);

  sh.write_buffer("__.", 17, 7, Color::DarkBlue);
  sh.write_buffer("((((((((((((((", 17, 14, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("|", 17, 45, Color::Green, Color::Cyan);
  sh.write_buffer("___  _  . ..", 17, 60, Color::DarkBlue);
  sh.write_buffer("//", 17, 74, Color::DarkGray, Color::LightGray);

  sh.write_buffer("(((((((((", 18, 15, Color::DarkYellow, Color::Yellow);
  sh.write_buffer("     ", 18, 24, Color::LightGray, Color::White);
  sh.write_buffer(".", 18, 29, Color::White, Color::Cyan);
  sh.write_buffer("|", 18, 45, Color::Green, Color::Cyan);

  sh.write_buffer(",", 19, 19, Color::White, Color::Cyan);
  sh.write_buffer("     ", 19, 20, Color::LightGray, Color::White);
  sh.write_buffer("_)", 19, 25, Color::White, Color::LightGray);
  sh.write_buffer("  ", 19, 27, Color::LightGray, Color::White);
  sh.write_buffer(")", 19, 29, Color::White, Color::Cyan);
  sh.write_buffer("|", 19, 45, Color::Green, Color::Cyan);

  sh.write_buffer("~~'", 20, 4, Color::White, Color::Cyan);
  sh.write_buffer(" ", 20, 7, Color::LightGray, Color::White);
  sh.write_buffer("`_", 20, 8, Color::White, Color::Cyan);
  sh.write_buffer("(,", 20, 18, Color::White, Color::Cyan);
  sh.write_buffer("   ", 20, 20, Color::LightGray, Color::White);
  sh.write_buffer("_)", 20, 23, Color::White, Color::LightGray);
  sh.write_buffer("      ", 20, 25, Color::LightGray, Color::White);
  sh.write_buffer(")", 20, 31, Color::White, Color::Cyan);
  sh.write_buffer(",------.", 20, 49, Color::DarkMagenta, Color::Magenta);

  sh.write_buffer("'", 21, 3, Color::White, Color::Cyan);
  sh.write_buffer("       ", 21, 4, Color::LightGray, Color::White);
  sh.write_buffer(".", 21, 11, Color::White, Color::Cyan);
  sh.write_buffer("`", 21, 20, Color::LightGray, Color::DarkCyan);
  sh.write_buffer("~~~~~=---", 21, 21, Color::DarkGray, Color::LightGray);
  sh.write_buffer("'", 21, 30, Color::LightGray, Color::DarkCyan);
  sh.write_buffer("__", 21, 43, Color::DarkMagenta);
  sh.write_buffer("A", 21, 45, Color::DarkMagenta, Color::Magenta);
  sh.write_buffer("__", 21, 46, Color::DarkMagenta);
  sh.write_buffer("/", 21, 48, Color::DarkMagenta, Color::Magenta);
  sh.write_buffer("_", 21, 49, Color::DarkGray, Color::Cyan);
  sh.write_buffer(")(", 21, 50, Color::LightGray, Color::DarkCyan);
  sh.write_buffer("_", 21, 52, Color::DarkGray, Color::Cyan);
  sh.write_buffer(")(", 21, 53, Color::LightGray, Color::DarkCyan);
  sh.write_buffer("_", 21, 55, Color::DarkGray, Color::Cyan);
  sh.write_buffer(")", 21, 56, Color::LightGray, Color::DarkCyan);
  sh.write_buffer("\\", 21, 57, Color::DarkMagenta, Color::Magenta);
  sh.write_buffer("_____", 21, 58, Color::DarkMagenta);
  sh.write_buffer("___ _", 21, 66, Color::DarkBlue);

  sh.write_buffer("_(", 22, 1, Color::White, Color::Cyan);
  sh.write_buffer("     ", 22, 3, Color::LightGray, Color::White);
  sh.write_buffer("_)", 22, 8, Color::White, Color::LightGray);
  sh.write_buffer("  ", 22, 10, Color::LightGray, Color::White);
  sh.write_buffer(")", 22, 12, Color::White, Color::Cyan);
  sh.write_buffer("/", 22, 42, Color::DarkMagenta, Color::Cyan);
  sh.write_buffer("  ________________  ", 22, 43, Color::DarkMagenta, Color::Magenta);
  sh.write_buffer("\\", 22, 63, Color::DarkMagenta, Color::Cyan);
  sh.write_buffer(".___", 22, 65, Color::DarkBlue);

  sh.write_buffer("(", 23, 0, Color::White, Color::Cyan);
  sh.write_buffer("  ", 23, 1, Color::LightGray, Color::White);
  sh.write_buffer("`", 23, 3, Color::White, Color::LightGray);
  sh.write_buffer("      ", 23, 4, Color::LightGray, Color::White);
  sh.write_buffer("'", 23, 10, Color::White, Color::LightGray);
  sh.write_buffer("  ", 23, 11, Color::LightGray, Color::White);
  sh.write_buffer(")", 23, 13, Color::White, Color::Cyan);
  sh.write_buffer("\\", 23, 42, Color::DarkMagenta, Color::Cyan);
  sh.write_buffer("_____________________", 23, 42, Color::DarkMagenta, Color::Magenta);
  sh.write_buffer("/", 23, 63, Color::DarkMagenta, Color::Cyan);
  sh.write_buffer(".._ __", 23, 66, Color::DarkBlue);

  sh.write_buffer("`", 24, 1, Color::LightGray, Color::DarkCyan);
  sh.write_buffer("_'`~~", 24, 2, Color::DarkGray, Color::LightGray);
  sh.write_buffer("____ ", 24, 7, Color::Black, Color::LightGray);
  
  ASCII_Fonts::draw_text(sh, font_data, font_colors, "Pilot Episode", 24, 6, ASCII_Fonts::Font::Avatar);
  //sh.replace_bg_color(Color::Yellow, ttl::Rectangle { 25, 6, 69, 4 });
  sh.replace_bg_color(Color::Yellow, ttl::Rectangle { 25, 6, 4, 28 });
  sh.replace_bg_color(Color::Yellow, ttl::Rectangle { 25, 36, 4, 39 });

  sh.write_buffer("(c) 2022 - 2024", 29, 64, Color::White);
  
  sh.write_buffer("Press space-bar to continue...", 29, 25, Color::White);
}

