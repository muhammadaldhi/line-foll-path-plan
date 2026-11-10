import re
from PIL import Image
import os

def parse_header(file_path):
    print(f"Reading {file_path}...")
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Match const uint16_t name[] PROGMEM = { ... };
    arrays = re.findall(r'const uint16_t (\w+)\[\] PROGMEM = \{(.*?)\};', content, re.DOTALL)
    
    output_dir = 'mockups'
    os.makedirs(output_dir, exist_ok=True)
    
    image_paths = {}
    
    for name, data_str in arrays:
        print(f"Processing {name}...")
        # Clean data: remove comments, newlines, spaces
        data_str = re.sub(r'//.*', '', data_str)
        hex_values = re.findall(r'0x[0-9a-fA-F]+', data_str)
        
        if len(hex_values) != 240 * 240:
            print(f"Warning: {name} has {len(hex_values)} pixels, expected {240*240}")
            continue
            
        img = Image.new('RGB', (240, 240))
        pixels = []
        
        for h in hex_values:
            val = int(h, 16)
            # RGB565 to RGB888
            r = ((val >> 11) & 0x1F) << 3
            g = ((val >> 5) & 0x3F) << 2
            b = (val & 0x1F) << 3
            pixels.append((r, g, b))
            
        img.putdata(pixels)
        save_path = os.path.join(output_dir, f"{name}.png")
        img.save(save_path)
        image_paths[name] = save_path
        print(f"Saved {save_path}")
        
    return image_paths

if __name__ == "__main__":
    parse_header('uiMain.h')
