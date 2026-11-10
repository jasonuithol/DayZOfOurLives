#!/usr/bin/env python3
"""Draws the Workshop title image, DayZOfOurLives/preview.png (1024x1024, under 1 MB):
a soap-opera title card - an hourglass running out over a ruined skyline at dusk.

    python3 tools/make_preview.py
"""
import math, os, random
from PIL import Image, ImageDraw, ImageFilter, ImageFont

S = 2048                      # drawn at twice the size, scaled down for smooth edges
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'DayZOfOurLives', 'preview.png')
FONTS = '/usr/share/fonts'
rng = random.Random(1965)

def font(name, size):
    for root, _, files in os.walk(FONTS):
        if name in files:
            return ImageFont.truetype(os.path.join(root, name), size)
    raise SystemExit('font not found: ' + name)

def lerp(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(len(a)))

img = Image.new('RGB', (S, S))
px = img.load()
# dusk sky: deep violet at the top, burning orange on the horizon
top, mid, low = (18, 14, 34), (92, 36, 58), (232, 118, 52)
for y in range(S):
    t = y / S
    c = lerp(top, mid, t / 0.55) if t < 0.55 else lerp(mid, low, min(1, (t - 0.55) / 0.25))
    for x in range(S):
        px[x, y] = c
d = ImageDraw.Draw(img)

# a low sun behind the skyline
glow = Image.new('L', (S, S))
ImageDraw.Draw(glow).ellipse((S * 0.30, S * 0.62, S * 0.70, S * 1.02), fill=255)
glow = glow.filter(ImageFilter.GaussianBlur(120))
img = Image.composite(Image.new('RGB', (S, S), (255, 196, 110)), img, glow.point(lambda v: v * 0.6))
d = ImageDraw.Draw(img)

# ruined skyline: blocks with broken tops, a church, a water tower, power lines
horizon = int(S * 0.80)
x = 0
while x < S:
    w = rng.randint(60, 170)
    h = rng.randint(80, 330) if abs(x - S / 2) > 260 else rng.randint(40, 120)
    top_y = horizon - h
    pts = [(x, horizon), (x, top_y)]
    for k in range(rng.randint(1, 4)):        # broken roofline
        pts.append((x + w * (k + 1) / 5, top_y + rng.randint(-25, 45)))
    pts += [(x + w, top_y + rng.randint(0, 30)), (x + w, horizon)]
    d.polygon(pts, fill=(22, 12, 20))
    for _ in range(rng.randint(0, 5)):        # a few lit windows
        wx, wy = x + rng.randint(10, max(11, w - 20)), top_y + rng.randint(30, max(31, h - 20))
        d.rectangle((wx, wy, wx + 10, wy + 14), fill=(250, 170, 70))
    x += w + rng.randint(-10, 25)
cx = int(S * 0.14)                            # church with an onion dome
d.rectangle((cx, horizon - 300, cx + 70, horizon), fill=(22, 12, 20))
d.ellipse((cx - 20, horizon - 400, cx + 90, horizon - 290), fill=(22, 12, 20))
d.polygon([(cx + 35, horizon - 470), (cx + 15, horizon - 390), (cx + 55, horizon - 390)], fill=(22, 12, 20))
wt = int(S * 0.86)                            # water tower
d.ellipse((wt - 80, horizon - 420, wt + 80, horizon - 330), fill=(22, 12, 20))
for lx in (-55, 55):
    d.line((wt + lx, horizon - 350, wt + lx * 1.3, horizon), fill=(22, 12, 20), width=10)
d.rectangle((0, horizon, S, S), fill=(14, 8, 14))
for k in range(3):                            # sagging power line
    y0 = horizon - 520 + k * 22
    d.line([(px_, y0 + 60 * math.sin(math.pi * px_ / S)) for px_ in range(0, S + 1, 16)], fill=(28, 16, 26), width=4)

# the hourglass
hx, hy, hw, hh = S // 2, int(S * 0.47), 300, 760    # centre, half width, full height
top_cap, bot_cap = hy - hh // 2, hy + hh // 2
def bulb(y):                                  # half width of the glass at height y: round bulbs
    u = min(1, abs(y - hy) / (hh / 2 - 40))   # 0 at the neck, 1 at the end plates
    w = math.sin(min(u / 0.72, 1) * math.pi / 2) ** 0.85
    w *= 1 - 0.18 * max(0, (u - 0.72) / 0.28) ** 1.5   # curving back in towards the plates
    return 16 + (hw - 16) * w
glass = Image.new('RGBA', (S, S), (0, 0, 0, 0))
gd = ImageDraw.Draw(glass)
left = [(hx - bulb(y), y) for y in range(top_cap + 40, bot_cap - 40, 4)]
right = [(hx + bulb(y), y) for y in range(bot_cap - 41, top_cap + 39, -4)]
gd.polygon(left + right, fill=(200, 225, 255, 46), outline=(235, 245, 255, 150))
# sand: what's left in the top, the stream, the heap below
sand = (236, 186, 96, 255)
top_sand = [(hx - bulb(y) + 10, y) for y in range(hy - 210, hy - 18, 4)] + \
           [(hx + bulb(y) - 10, y) for y in range(hy - 18, hy - 211, -4)]
gd.polygon(top_sand, fill=sand)
gd.ellipse((hx - bulb(hy - 210) + 10, hy - 240, hx + bulb(hy - 210) - 10, hy - 180), fill=sand)
gd.rectangle((hx - 5, hy - 20, hx + 5, bot_cap - 200), fill=sand)
heap = [(hx - bulb(y) + 10, y) for y in range(bot_cap - 45, bot_cap - 150, -4)]
heap += [(hx - 90, bot_cap - 175), (hx, bot_cap - 215), (hx + 90, bot_cap - 175)]
heap += [(hx + bulb(y) - 10, y) for y in range(bot_cap - 150, bot_cap - 44, 4)]
gd.polygon(heap, fill=sand)
for _ in range(60):                           # grains in the air
    gy = rng.uniform(hy, bot_cap - 210)
    gx = hx + rng.gauss(0, 7)
    gd.ellipse((gx - 3, gy - 3, gx + 3, gy + 3), fill=sand)
gd.line([(hx - bulb(y) + 26, y) for y in range(top_cap + 90, hy - 60, 4)], fill=(255, 255, 255, 120), width=10)  # highlight
gd.line([(hx + bulb(y) - 30, y) for y in range(hy + 70, bot_cap - 240, 4)], fill=(255, 255, 255, 70), width=7)
img.paste(glass, (0, 0), glass)
d = ImageDraw.Draw(img)
wood, brass = (58, 30, 18), (196, 150, 72)
for cap in (top_cap, bot_cap):                # end plates and posts
    d.rounded_rectangle((hx - hw - 70, cap - 34, hx + hw + 70, cap + 34), radius=18, fill=wood, outline=brass, width=8)
for sx in (-1, 1):
    d.rounded_rectangle((hx + sx * (hw + 30) - 16, top_cap, hx + sx * (hw + 30) + 16, bot_cap), radius=12, fill=wood, outline=brass, width=6)

# title
def centred(text, f, y, fill, shadow=(0, 0, 0), spread=10):
    w = d.textlength(text, font=f)
    sh = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    ImageDraw.Draw(sh).text(((S - w) / 2, y + 8), text, font=f, fill=shadow + (220,))
    sh = sh.filter(ImageFilter.GaussianBlur(spread))
    img.paste(sh, (0, 0), sh)
    ImageDraw.Draw(img).text(((S - w) / 2, y), text, font=f, fill=fill)
centred('DayZ', font('P052-Bold.otf', 330), 40, (250, 236, 210))
centred('of Our Lives', font('Z003-MediumItalic.otf', 250), 350, (244, 196, 104))
d = ImageDraw.Draw(img)

# roles along the bottom
roles = 'POLICE  ·  DOCTOR  ·  FIREFIGHTER  ·  HUNTER  ·  PRISONER  ·  EXECUTIVE  ·  SOLDIER  ·  PILOT'
f = font('P052-Bold.otf', 50)
w = d.textlength(roles, font=f)
if w > S - 120:
    f = font('P052-Bold.otf', int(50 * (S - 120) / w))
centred(roles, f, int(S * 0.875), (236, 214, 190), spread=6)
centred('everybody was somebody', font('P052-Italic.otf', 70), int(S * 0.925), (214, 160, 120), spread=6)

# film grain and vignette
noise = Image.effect_noise((S, S), 22).convert('RGB')
img = Image.blend(img, noise, 0.05)
vig = Image.new('L', (S, S), 0)
ImageDraw.Draw(vig).ellipse((-S * 0.2, -S * 0.2, S * 1.2, S * 1.2), fill=255)
vig = vig.filter(ImageFilter.GaussianBlur(220))
img = Image.composite(img, Image.new('RGB', (S, S), (6, 2, 8)), vig)

img = img.resize((1024, 1024), Image.LANCZOS)
img = img.quantize(colors=256, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.FLOYDSTEINBERG).convert('RGB')
img.save(OUT, optimize=True)
print('%s: %d KB' % (os.path.normpath(OUT), os.path.getsize(OUT) // 1024))
