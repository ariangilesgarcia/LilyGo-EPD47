import click
from PIL import Image

@click.command()
@click.argument('input_image')
@click.argument('output_header')
def convert_image(input_image, output_header):
    """Convert an image to the format required for the LilyGo T5 4.7" e-ink display."""

    # Load and convert the image
    img = Image.open(input_image)
    img = img.convert('1')  # Convert image to 1-bit pixels (black & white)
    width, height = img.size  # Get the dimensions of the image

    # Convert the image to the required format
    img_data = img.tobytes()

    # Prepare data for .h file
    header_data = f"const uint32_t image_width = {width};\n"
    header_data += f"const uint32_t image_height = {height};\n"
    header_data += f"const uint8_t image_data[{len(img_data)}] = {{\n    "

    for i, byte in enumerate(img_data):
        if i % 12 == 0 and i > 0:
            header_data += "\n    "
        header_data += f"0x{byte:02X}, "

    header_data = header_data.rstrip(", ") + "\n};\n"

    # Write the .h file
    with open(output_header, 'w') as f:
        f.write(header_data)

    click.echo(f"Image converted and saved to {output_header}")

if __name__ == '__main__':
    convert_image()
