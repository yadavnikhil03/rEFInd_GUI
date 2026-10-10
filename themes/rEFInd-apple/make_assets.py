import os
from PIL import Image, ImageDraw, ImageFont

def create_squircle_mask(size, radius):
    mask = Image.new('L', size, 0)
    draw = ImageDraw.Draw(mask)
    draw.rounded_rectangle((0, 0, size[0], size[1]), radius=radius, fill=255)
    return mask

def create_selection(filename, size, radius):
    img = Image.new('RGBA', size, (0, 0, 0, 0))
    overlay = Image.new('RGBA', size, (255, 255, 255, 30))
    
    mask = create_squircle_mask(size, radius)
    img.paste(overlay, (0, 0), mask)
    
    draw = ImageDraw.Draw(img)
    draw.rounded_rectangle((0, 0, size[0]-1, size[1]-1), radius=radius, outline=(255, 255, 255, 100), width=1)
    
    img.save(filename)

def create_os_icon(filename, size, radius, text, color):
    img = Image.new('RGBA', size, (0, 0, 0, 0))
    bg = Image.new('RGBA', size, color)
    mask = create_squircle_mask(size, radius)
    img.paste(bg, (0, 0), mask)
    
    gloss = Image.new('RGBA', size, (255, 255, 255, 20))
    gloss_mask = Image.new('L', size, 0)
    gloss_draw = ImageDraw.Draw(gloss_mask)
    gloss_draw.ellipse((-size[0]*0.5, -size[1]*0.5, size[0]*1.5, size[1]*0.4), fill=255)
    img.paste(gloss, (0,0), gloss_mask)

    draw = ImageDraw.Draw(img)
    try:
        font = ImageFont.truetype("arial.ttf", int(size[1]*0.4))
    except:
        font = ImageFont.load_default()
    
    bbox = draw.textbbox((0, 0), text, font=font)
    w = bbox[2] - bbox[0]
    h = bbox[3] - bbox[1]
    draw.text(((size[0]-w)/2, (size[1]-h)/2), text, font=font, fill=(255, 255, 255, 255))
    
    img.save(filename)

out_dir = r"c:\Users\nikhi\Downloads\rEFInd_GUI\themes\rEFInd-apple\icons"
os.makedirs(out_dir, exist_ok=True)

create_selection(os.path.join(out_dir, "selection_big.png"), (144, 144), 32)
create_selection(os.path.join(out_dir, "selection_small.png"), (64, 64), 16)

create_os_icon(os.path.join(out_dir, "os_win.png"), (128, 128), 28, "Win", (0, 120, 215, 255))
create_os_icon(os.path.join(out_dir, "os_mac.png"), (128, 128), 28, "Mac", (80, 80, 80, 255))
create_os_icon(os.path.join(out_dir, "os_linux.png"), (128, 128), 28, "Lin", (233, 84, 32, 255))
create_os_icon(os.path.join(out_dir, "os_ubuntu.png"), (128, 128), 28, "Ubu", (233, 84, 32, 255))
