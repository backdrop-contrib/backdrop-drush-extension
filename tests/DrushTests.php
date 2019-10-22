<?php
/**
 * @file
 * Initial test suite for Backdrop Drush Extension.
 */

use PHPUnit\Framework\TestCase;

class DrushTests extends TestCase {
  /**
   * Test drush ctl command.
   */
  public function testDrushCtl() {
    $output = shell_exec('drush ctl');
    $this->assertStringContainsString('Type', $output);
    $this->assertStringContainsString('page', $output);
  }

  /**
   * Test drush st command.
   */
  public function testDrushSt() {
    exec('drush st', $output);
    $this->assertStringContainsString('Backdrop version', $output[0]);
    $this->assertStringContainsString('Backdrop bootstrap', $output[7]);
    $this->assertStringContainsString('Successful', $output[7]);
    $this->assertStringContainsString('PHP OS', $output[10]);
    $this->assertStringContainsString('Backdrop Settings File', $output[18]);
  }

  /**
   * Test drush dl devel command.
   */
  public function testPmDownload() {
    exec("drush dl devel -n", $output);
    $this->assertStringContainsString('Success:', $output[1]);
    $this->assertStringContainsString('devel', $output[1]);
    $this->assertDirectoryExists('/app/web/modules/devel');
    // Clean up.
    exec('rm -rf /app/web/modules/devel');
  }
}
