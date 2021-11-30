/*
 * Copyright 2011 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_GFDM_API_H
#define INCLUDED_GFDM_API_H

#include <gnuradio/attributes.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

/*
 * fmtlib lacks a std::complex formatter.
 * This one is ripped off of
 * https://github.com/fmtlib/fmt/issues/1467#issuecomment-653094376
 *
 * Just copypasta. I included `fmt/format.h` as well.
 * The issue thread claims "considering that complex is somewhat niche"
 * https://github.com/fmtlib/fmt/issues/1467#issuecomment-563287076
 *
 * So: welcome to this niche!
 * 
 * This is getting better... it is broken as of fmtlib 8.x ...
 */

// template <typename T, typename Char>
// struct fmt::formatter<std::complex<T>, Char> : public fmt::formatter<T, Char> {
//     typedef fmt::formatter<T, Char> base;
//     enum style { expr, star, pair } style_ = expr;
//     internal::dynamic_format_specs<Char> specs_;
//     constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
//     {
//         using handler_type = internal::dynamic_specs_handler<format_parse_context>;
//         auto type = internal::type_constant<T, Char>::value;
//         internal::specs_checker<handler_type> handler(handler_type(specs_, ctx), type);
//         auto it = ctx.begin();
//         /*if (it != ctx.end())*/ { // uncomment to remove SEGV
//             switch (*it) {
//             case '$':
//                 style_ = style::expr;
//                 ctx.advance_to(++it);
//                 break;
//             case '*':
//                 style_ = style::star;
//                 ctx.advance_to(++it);
//                 break;
//             case ',':
//                 style_ = style::pair;
//                 ctx.advance_to(++it);
//                 break;
//             default:
//                 break;
//             }
//         }
//         if (style_ != style::pair)
//             parse_format_specs(ctx.begin(), ctx.end(), handler);
//         // todo: fixup alignment
//         return base::parse(ctx);
//     }
//     template <typename FormatCtx>
//     auto format(const std::complex<T>& x, FormatCtx& ctx) -> decltype(ctx.out())
//     {
//         format_to(ctx.out(), "(");
//         if (x.real() || !x.imag())
//             base::format(x.real(), ctx);
//         if (x.imag()) {
//             if (style_ == style::pair)
//                 format_to(ctx.out(), ",");
//             else if (x.real() && x.imag() >= 0 && specs_.sign != sign::plus)
//                 format_to(ctx.out(), "+");
//             base::format(x.imag(), ctx);
//             if (style_ != style::pair) {
//                 if (style_ == style::star)
//                     format_to(ctx.out(), "*i");
//                 else
//                     format_to(ctx.out(), "i");
//                 if (std::is_same<typename std::decay<T>::type, float>::value)
//                     format_to(ctx.out(), "f");
//                 if (std::is_same<typename std::decay<T>::type, long double>::value)
//                     format_to(ctx.out(), "l");
//             }
//         }
//         return format_to(ctx.out(), ")");
//     }
// };

#ifdef gnuradio_gfdm_EXPORTS
#define GFDM_API __GR_ATTR_EXPORT
#else
#define GFDM_API __GR_ATTR_IMPORT
#endif

#endif /* INCLUDED_GFDM_API_H */
