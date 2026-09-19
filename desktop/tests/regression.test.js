import { describe, it } from 'node:test';
import assert from 'node:assert';
import fs from 'node:fs';
import path from 'node:path';

describe('Regression Tests - UI Structure & Design Specs', () => {
  const htmlPath = path.resolve(process.cwd(), 'index.html');
  const cssPath = path.resolve(process.cwd(), 'src/style.css');

  const htmlContent = fs.readFileSync(htmlPath, 'utf8');
  const cssContent = fs.readFileSync(cssPath, 'utf8');

  it('should maintain correct header buttons order: Painel -> Gravar tela -> Fullscreen', () => {
    const painelIdx = htmlContent.indexOf('id="btnToggleSidebar"');
    const recordIdx = htmlContent.indexOf('id="btnRecordScreen"');
    const fullscreenIdx = htmlContent.indexOf('id="btnFullscreen"');

    assert.ok(painelIdx !== -1, 'Painel button must exist');
    assert.ok(recordIdx !== -1, 'Gravar tela button must exist');
    assert.ok(fullscreenIdx !== -1, 'Fullscreen button must exist');

    assert.ok(painelIdx < recordIdx, 'Painel must precede Gravar tela');
    assert.ok(recordIdx < fullscreenIdx, 'Gravar tela must precede Fullscreen');
  });

  it('should not contain deprecated btnSnapshot in header actions', () => {
    assert.strictEqual(htmlContent.includes('id="btnSnapshot"'), false, 'btnSnapshot must be removed');
  });

  it('should contain Export Sidebar with required controls', () => {
    assert.ok(htmlContent.includes('id="exportSidebar"'), '#exportSidebar element must exist');
    assert.ok(htmlContent.includes('id="exportFormatGif"'), '#exportFormatGif must exist');
    assert.ok(htmlContent.includes('id="exportFormatMp4"'), '#exportFormatMp4 must exist');
    assert.ok(htmlContent.includes('id="btnDownloadExport"'), '#btnDownloadExport must exist');
  });

  it('should maintain standard button visual style for Gravar tela matching other header buttons', () => {
    assert.ok(htmlContent.includes('class="btn btn-secondary btn-record"'), 'Gravar tela button must use standard btn-secondary styling');
    assert.ok(cssContent.includes('.btn-record.recording'), '.btn-record.recording style must exist');
  });
});
