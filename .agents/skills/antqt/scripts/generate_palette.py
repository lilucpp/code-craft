#!/usr/bin/env python3
"""
Ant Design HSB 10-Step Palette Generator for Qt (QSS & QML).
Generates light and dark ramps from a seed color following Ant Design Color Algorithm.
"""

import sys
import colorsys
import argparse

def hex_to_rgb(hex_str: str):
    hex_str = hex_str.lstrip('#')
    return tuple(int(hex_str[i:i+2], 16) / 255.0 for i in (0, 2, 4))

def rgb_to_hex(r: float, g: float, b: float) -> str:
    return f"#{int(round(r * 255)):02X}{int(round(g * 255)):02X}{int(round(b * 255)):02X}"

def get_hue(h: float, i: int, is_light: bool) -> float:
    hue_step = 2.0 / 360.0
    if round(h * 360) >= 60 and round(h * 360) <= 240:
        hue = h - hue_step * (i if is_light else -i)
    else:
        hue = h + hue_step * (i if is_light else -i)
    if hue < 0:
        hue += 1.0
    elif hue >= 1.0:
        hue -= 1.0
    return hue

def get_saturation(s: float, i: int, is_light: bool) -> float:
    if s == 0:
        return 0
    if is_light:
        sat_step = 0.16
        sat_step2 = 0.05
        ret = s - sat_step if i == 5 else s - sat_step - (i - 1) * sat_step2
    else:
        sat_step = 0.07
        ret = s + i * sat_step
    return max(0.06, min(1.0, ret))

def get_value(v: float, i: int, is_light: bool) -> float:
    if is_light:
        val_step = 0.05
        ret = v + i * val_step
    else:
        val_step = 0.15
        ret = v - i * val_step
    return max(0.0, min(1.0, ret))

def generate_palette(primary_hex: str):
    r, g, b = hex_to_rgb(primary_hex)
    h, s, v = colorsys.rgb_to_hsv(r, g, b)
    
    # 6 is the primary seed
    colors = [None] * 10
    colors[5] = primary_hex.upper()
    
    # Light steps: 5, 4, 3, 2, 1
    for idx in range(4, -1, -1):
        step = 5 - idx
        nh = get_hue(h, step, True)
        ns = get_saturation(s, step, True)
        nv = get_value(v, step, True)
        nr, ng, nb = colorsys.hsv_to_rgb(nh, ns, nv)
        colors[idx] = rgb_to_hex(nr, ng, nb)
        
    # Dark steps: 7, 8, 9, 10
    for idx in range(6, 10):
        step = idx - 5
        nh = get_hue(h, step, False)
        ns = get_saturation(s, step, False)
        nv = get_value(v, step, False)
        nr, ng, nb = colorsys.hsv_to_rgb(nh, ns, nv)
        colors[idx] = rgb_to_hex(nr, ng, nb)
        
    return {
        "1": colors[0],   # Selected Bg / Hover Lightest
        "2": colors[1],
        "3": colors[2],
        "4": colors[3],
        "5": colors[4],   # Primary Hover
        "6": colors[5],   # Base Primary
        "7": colors[6],   # Primary Active / Pressed
        "8": colors[7],
        "9": colors[8],
        "10": colors[9]
    }

def print_tokens(seed_hex: str):
    palette = generate_palette(seed_hex)
    print(f"/* Ant Design 10-step Palette for Seed: {seed_hex} */")
    for k, v in palette.items():
        print(f"  --ant-primary-{k}: {v};")
    print("\n/* Common Semantic Mappings */")
    print(f"  --ant-primary-hover: {palette['5']};")
    print(f"  --ant-primary-base:  {palette['6']};")
    print(f"  --ant-primary-active:{palette['7']};")
    print(f"  --ant-primary-bg:    {palette['1']};")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Ant Design HSB 10-Step Palette Generator for Qt")
    parser.add_argument("--seed", default="#1677FF", help="Primary seed hex color (default: #1677FF)")
    args = parser.parse_args()
    print_tokens(args.seed)
