import math

class SVGGenerator:
    """SVGを作るためのクラス"""

    def __init__(self, width_mm=100, height_mm=100):
        self.width_mm = width_mm
        self.height_mm = height_mm
        self.elements = []

    def circle(self, radius_mm):
        """半径radius_mmのなるべく細い円を書く"""
        self.elements.append(
            f'<circle cx="{self.width_mm / 2}" cy="{self.height_mm / 2}" r="{radius_mm}" stroke="black" stroke-width="0.1" fill="none" />'
        )

    def sector_ring(self, outer_radius_mm, inner_radius_mm, start_degree, end_degree):
        """outer_radius_mmの円から、inner_radius_mmの円を切り取り、start_degreeからend_degreeまでを取り出した扇形環を描く"""

        def polar_to_cartesian(radius, angle_in_degrees):
            angle_in_radians = (angle_in_degrees - 90) * math.pi / 180.0
            return (
                (self.width_mm / 2) + (radius * math.cos(angle_in_radians)),
                (self.height_mm / 2) + (radius * math.sin(angle_in_radians))
            )

        start_outer = polar_to_cartesian(outer_radius_mm, end_degree)
        end_outer = polar_to_cartesian(outer_radius_mm, start_degree)
        start_inner = polar_to_cartesian(inner_radius_mm, end_degree)
        end_inner = polar_to_cartesian(inner_radius_mm, start_degree)

        large_arc_flag = "1" if end_degree - start_degree > 180 else "0"

        path_data = [
            f"M {start_outer[0]} {start_outer[1]}",
            f"A {outer_radius_mm} {outer_radius_mm} 0 {large_arc_flag} 0 {end_outer[0]} {end_outer[1]}",
            f"L {end_inner[0]} {end_inner[1]}",
            f"A {inner_radius_mm} {inner_radius_mm} 0 {large_arc_flag} 1 {start_inner[0]} {start_inner[1]}",
            "Z"
        ]
        self.elements.append(f'<path d="{" ".join(path_data)}" stroke="none" stroke-width="0.1" fill="black"/>')

    def indicator(self, inner_radius_mm, pos_digree):
        """inner_radius_mm、0度の位置に大きさ2mm程度の三角を描く"""
        size_mm = 2

        def polar_to_cartesian(radius, angle_in_degrees):
            angle_in_radians = (angle_in_degrees - 90) * math.pi / 180.0
            return (
                (self.width_mm / 2) + (radius * math.cos(angle_in_radians)),
                (self.height_mm / 2) + (radius * math.sin(angle_in_radians))
            )

        # Tip of the triangle, pointing towards the center
        p1 = polar_to_cartesian(inner_radius_mm, 0 + pos_digree)

        # Base of the triangle is further out
        base_radius = inner_radius_mm + size_mm

        half_base_width = size_mm / 2
        half_base_angle_rad = half_base_width / base_radius
        half_base_angle_deg = half_base_angle_rad * 180 / math.pi

        p2 = polar_to_cartesian(base_radius, -half_base_angle_deg + pos_digree)
        p3 = polar_to_cartesian(base_radius, half_base_angle_deg + pos_digree)

        self.elements.append(
            f'<polygon points="{p1[0]},{p1[1]} {p2[0]},{p2[1]} {p3[0]},{p3[1]}" fill="black" />'
        )

    def export_to_svg(self):
        """描いたデータをsvgにして出力する。"""
        svg_elements = "\n".join(self.elements)
        return (
            f'<svg width="{self.width_mm}mm" height="{self.height_mm}mm" viewBox="0 0 {self.width_mm} {self.height_mm}" xmlns="http://www.w3.org/2000/svg">\n'
            + svg_elements
            + "\n</svg>"
        )

encoders = [
    {
        "name": "one_dial",
        "bits": 6,
        "degrees": [
            0,  # 最初の遊び
        ]+[
            (i * 23 / 3 + 20) for i in range(33+2)
        ] + [320],
        "indicator": 23/3*.5+20 + 23/3+5,
    }
]


for encoder in encoders:
    gen = SVGGenerator()
    bits = encoder["bits"]
    templ = encoder["degrees"]
    gen.circle(8)
    gen.circle(8+5*bits)
    gen.indicator(8+5*bits, encoder["indicator"])

    for i in range(len(templ)-1):
        num = i+1
        gray = num ^ (num>>1)
        s = f'{gray:0{bits}b}'
        # print(s)
        for j in range(bits):
            if (s[bits-j-1] == "1"):
                gen.sector_ring(8+5+5*j, 8+5*j, templ[i], templ[i+1])

    with open(f"{encoder["name"]}.svg", "w") as f:
        f.write(gen.export_to_svg())
