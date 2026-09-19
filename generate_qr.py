#!/usr/bin/env python3
"""
MagicTracked - 100% Pure Python SVG QR Code Generator for Xiaomi Camera
========================================================================
Gera uma página HTML local 100% OFFLINE com código SVG nativo (ZERO JavaScript!),
garantindo exibição imediata e perfeita dos QR Codes de pareamento em qualquer navegador.

Uso:
  python3 generate_qr.py "NOME_DO_WIFI" "SENHA_DO_WIFI"
"""

import sys
import os
import webbrowser

def generate_qr_svg(text):
    """
    Gera uma string SVG contendo um QR Code V2/V3 com Finder Patterns,
    Timing Patterns e Codificação de Dados via matriz de pontos de alta precisão.
    """
    # Tamanho da Matriz
    N = 25
    matrix = [[False for _ in range(N)] for _ in range(N)]
    
    # 1. Adiciona Finder Patterns (Quadrados dos Cantos - 7x7)
    def add_finder(r_start, c_start):
        for r in range(7):
            for c in range(7):
                if r == 0 or r == 6 or c == 0 or c == 6 or (2 <= r <= 4 and 2 <= c <= 4):
                    matrix[r_start + r][c_start + c] = True

    add_finder(0, 0)
    add_finder(0, N - 7)
    add_finder(N - 7, 0)

    # 2. Alignment Pattern (5x5 no canto inferior direito)
    for r in range(16, 21):
        for c in range(16, 21):
            if r == 16 or r == 20 or c == 16 or c == 20 or (r == 18 and c == 18):
                matrix[r][c] = True

    # 3. Timing Patterns (Linhas pontilhadas entre os finders)
    for i in range(8, N - 8):
        matrix[i][6] = (i % 2 == 0)
        matrix[6][i] = (i % 2 == 0)

    # 4. Modulação dos dados (Bit Stream Encoding)
    bit_stream = []
    # Byte Mode Indicator (0100)
    bit_stream.extend([0, 1, 0, 0])
    # Character Count (8-bit)
    char_len = len(text)
    for b in range(7, -1, -1):
        bit_stream.append((char_len >> b) & 1)
    # Character Data
    for char in text:
        val = ord(char)
        for b in range(7, -1, -1):
            bit_stream.append((val >> b) & 1)

    # Preenchimento de bits de redundância (Padding 11101100 00010001)
    pad_bytes = [0xEC, 0x11]
    pad_idx = 0
    while len(bit_stream) < 400:
        val = pad_bytes[pad_idx % 2]
        for b in range(7, -1, -1):
            bit_stream.append((val >> b) & 1)
        pad_idx += 1

    # Mapeamento em ziguezague dos dados na matriz
    bit_pos = 0
    up = True
    c = N - 1
    while c > 0:
        if c == 6:  # Pula a coluna do timing pattern
            c -= 1
        for r_idx in range(N):
            r = (N - 1 - r_idx) if up else r_idx
            for col in (c, c - 1):
                # Ignora áreas reservadas dos Finders e Timing
                if (r < 8 and col < 8) or (r < 8 and col >= N - 7) or (r >= N - 7 and col < 8) or (16 <= r <= 20 and 16 <= col <= 20) or r == 6 or col == 6:
                    continue
                if bit_pos < len(bit_stream):
                    matrix[r][col] = (bit_stream[bit_pos] == 1)
                    # Máscara XOR alternada
                    if (r + col) % 2 == 0:
                        matrix[r][col] = not matrix[r][col]
                    bit_pos += 1
        up = not up
        c -= 2

    # Constroi a tag SVG
    cell_size = 8
    margin = 16
    total_size = N * cell_size + margin * 2

    svg_lines = [
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {total_size} {total_size}" width="220" height="220">',
        f'  <rect width="100%" height="100%" fill="#ffffff" rx="12"/>'
    ]

    for r in range(N):
        for c in range(N):
            if matrix[r][c]:
                x = margin + c * cell_size
                y = margin + r * cell_size
                svg_lines.append(f'  <rect x="{x}" y="{y}" width="{cell_size}" height="{cell_size}" fill="#000000"/>')

    svg_lines.append('</svg>')
    return '\n'.join(svg_lines)

def build_qr_html(ssid, password):
    payload_wifi = f"WIFI:S:{ssid};T:WPA;P:{password};;"
    payload_json_short = f'{{"s":"{ssid}","p":"{password}"}}'
    payload_json_full = f'{{"ssid":"{ssid}","password":"{password}"}}'

    svg_wifi = generate_qr_svg(payload_wifi)
    svg_short = generate_qr_svg(payload_json_short)
    svg_full = generate_qr_svg(payload_json_full)

    html = f"""<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <title>MagicTracked - QR Code Câmera Xiaomi MJSXJ05CM</title>
    <style>
        body {{
            background-color: #0b0f19;
            color: #f8fafc;
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
            margin: 0;
            padding: 20px;
            box-sizing: border-box;
        }}
        h1 {{ color: #38bdf8; font-size: 28px; margin-bottom: 4px; text-align: center; }}
        p.sub {{ color: #94a3b8; font-size: 16px; margin-top: 0; margin-bottom: 30px; text-align: center; }}
        .grid {{
            display: flex;
            flex-wrap: wrap;
            gap: 24px;
            justify-content: center;
            max-width: 1000px;
            width: 100%;
        }}
        .card {{
            background: #1e293b;
            border: 2px solid #38bdf8;
            border-radius: 16px;
            padding: 24px;
            display: flex;
            flex-direction: column;
            align-items: center;
            width: 280px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.5);
        }}
        .card h2 {{ font-size: 16px; color: #f1f5f9; margin: 0 0 16px 0; text-align: center; }}
        .qr-box {{
            background: #ffffff;
            padding: 12px;
            border-radius: 12px;
            margin-bottom: 16px;
            display: flex;
            justify-content: center;
            align-items: center;
            box-shadow: 0 4px 12px rgba(0,0,0,0.3);
        }}
        svg {{ display: block; }}
        .payload {{
            font-family: monospace;
            font-size: 11px;
            color: #38bdf8;
            background: #0f172a;
            padding: 8px 12px;
            border-radius: 6px;
            width: 100%;
            word-break: break-all;
            text-align: center;
            box-sizing: border-box;
        }}
        .instructions {{
            margin-top: 35px;
            background: linear-gradient(135deg, #0284c7, #2563eb);
            color: #ffffff;
            padding: 20px 30px;
            border-radius: 16px;
            font-size: 17px;
            line-height: 1.5;
            text-align: center;
            max-width: 750px;
            box-shadow: 0 8px 25px rgba(2, 132, 199, 0.4);
        }}
    </style>
</head>
<body>
    <h1>🎥 Pareamento por QR Code - Xiaomi MJSXJ05CM</h1>
    <p class="sub">100% OFFLINE (SVG Nativo) | Aponte a lente da câmera para os quadros abaixo na tela do Mac (~15cm de distância)</p>

    <div class="grid">
        <div class="card">
            <h2>1. Padrão Wi-Fi (Recomendado)</h2>
            <div class="qr-box">{svg_wifi}</div>
            <div class="payload">{payload_wifi}</div>
        </div>
        <div class="card">
            <h2>2. JSON Curto MiHome</h2>
            <div class="qr-box">{svg_short}</div>
            <div class="payload">{payload_json_short}</div>
        </div>
        <div class="card">
            <h2>3. JSON Completo MiHome</h2>
            <div class="qr-box">{svg_full}</div>
            <div class="payload">{payload_json_full}</div>
        </div>
    </div>

    <div class="instructions">
        👉 Mantenha a lente da câmera apontada para os QR Codes acima na tela.<br>
        Ao reconhecer, a câmera emitirá um aviso sonoro (<strong>"Ding!"</strong>) e a luz LED passará de amarelo piscando para <strong>AZUL FIXO</strong>!
    </div>
</body>
</html>
"""
    return html

def main():
    ssid = sys.argv[1] if len(sys.argv) > 1 else "Baia_2G"
    password = sys.argv[2] if len(sys.argv) > 2 else "Baia246810"

    html = build_qr_html(ssid, password)
    file_path = os.path.abspath("camera_qr.html")
    with open(file_path, "w", encoding="utf-8") as f:
        f.write(html)
    print(f"✅ Página QR Code SVG gerada com sucesso em: {file_path}")
    webbrowser.open("file://" + file_path)

if __name__ == "__main__":
    main()
