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
    $output = shell_exec('drush st');
    $this->assertStringContainsString('Backdrop version', $output);
    $this->assertStringContainsString('Backdrop bootstrap', $output);
    $this->assertStringContainsString('Successful', $output);
    $this->assertStringContainsString('PHP OS', $output);
    $this->assertStringContainsString('Backdrop Settings File', $output);
  }

  /**
   * Test drush config-get command.
   */
  public function testDrushConfigGet() {
    $output = shell_exec('drush config-get system.core');
    $this->assertStringContainsString('user_admin_role', $output);
  }

  /**
   * Test drush config-set command.
   */
  public function testDrushConfigSet() {
    $output = shell_exec(
      'drush config-set system.core site_name blunderbus'
    );
    $this->assertStringContainsString('blunderbus', $output);
  }

  /**
   * Test drush state-set command.
   */
  public function testDrushStateSet() {
    $output = shell_exec(
      'drush state-set maintenance_mode 0'
    );
    $this->assertStringContainsString('0', $output);
    $this->assertStringContainsString('maintenance_mode', $output);
  }

  /**
   * Test drush version command.
   */
  public function testDrushVersion() {
    $output = shell_exec(
      'drush version'
    );
    $this->assertStringContainsString('proper version', $output);
    $this->assertStringContainsString('Extension version', $output);
  }

  /**
   * Test drush ws command.
   */
  public function testDrushWatchdogShow() {
    $output = shell_exec(
      'drush ws'
    );
    $this->assertStringContainsString('ID', $output);
    $this->assertStringContainsString('Date', $output);
    $this->assertStringContainsString('Type', $output);
  }
}
