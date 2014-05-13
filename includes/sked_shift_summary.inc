<?php

/**
 * @file
 * Include file to summarise monthly shifts and make table for reconciliation.
 */

// Define constants here.
//define('MAX_COLUMNS', 33);


/**
 * Monthly shift summary of work done by rads.
 */
function sked_shift_summary($year = '', $month = '') {
  
  $arrRads = _sked_active_rads();

  // @TODO. Use theme function.
  $strOutput = '<table><thead><tr><th>' . t('Rad') . '</th>';

  //$strOutput .= '<th>SL Full Day</th><th>SL Half Day</th><th>YCH Full Day</th><th>YCH 1/2</th><th>Davis</th><th>Vaughn</th><th>"Q"</th><th>Evening</th><th>Night Call</th><th>W/E YCH</th><th>W/E SL</th><th>"P"</th><th>Business 1/2 Day</th><th>IR Off hour Callbacks</th><th>Adjustments</th><th>Total worked FT</th>';

  $types = _sked_get_shift_types();
  foreach ($types as $key => $type) {
    $strOutput .= '<th>' . $type . '</th>';
  }
  
  $strOutput .= '</tr></thead>';

  $strOutput .= '<tbody>';

  $counter = 0;

  foreach ($arrRads as $key => $value) {

    $class = 'odd';
    if ($counter % 2):
      $class= 'even';
    endif;

    $strOutput .= '<tr class="' . $class . '"><td>' . $value . '</td>';

    $types = _sked_get_shift_types();

    foreach ($types as $index => $type) {
      $value = _sked_shift_type_tally($key, $year, $month, $index, $type);
      $strOutput .= '<td>' . $value . '</td>';
    }

    $strOutput .= '</tr>';

    $counter++;

  }

  $strOutput .= '</tbody></table>';

  return $strOutput;

  /*foreach ($arrRads as $key => $value) {
     $arrSingleRadShifts = _sked_shift_tally($key, $year, $month);
    
    $strOutput .= '<tr class="rad-heading"><td>' . $value . '</td><td>' . date('M Y', strtotime($year . '-' . $month)) .  '</td></tr>';

      $counter = 1;
      foreach ($arrSingleRadShifts as $key => $value) {
        $class = 'odd';
        if ($counter % 2):
          $class= 'even';
        endif;

        $strOutput .= '<tr class="' . $class . '"><td width="15%">' . $value['code'] . '</td><td width="85%">' . $value['tally'] . '</td></tr>';
        $counter++; 
      }
  }

  $strOutput .= '</table>';

  return $strOutput;
  */
  
}


/**
 * Figure out num shifts per type of shift.
 * Example: South Lake Full Day, South Lake Half Day, Evening, Call etc.
 * @param
 *   $uid. Integer. Rad Drupal User ID.
 * @param
 *   $year. Integer.
 * @param
 *   $month. Integer.
 * @param
 *   $type_int. Integer. Index of shift type from _sked_get_shift_types(). 
 * @param
 *   $type. String. Shift type.
 * @return
 *   Integer. Number of shifts worked.
 */
function _sked_shift_type_tally($uid, $year, $month, $type_int, $type) {

  $max_days = date('t', strtotime($year . '-' . $month . '-01'));

  $start = $year . '-' . $month . '-' . '01';
  $end = $year . '-' . $month . '-' . $max_days; 

  // @TODO. This needs to account for all the types. (0 to 16 or so!)
 /* if ($type_int == 0) {
    $codes = array('S1', 'S4', 'S5', 'S7');
  }
  else {
    $codes = array('C1', 'C4', 'C5', 'C7');
  }*/

  // @TODO. This needs to account for all the types and their permutations. (0 to 15 or so!)
  switch ($type_int) {
    case 0: // SL Full Day
      $codes = array('%S%'); //array('S1', 'S4', 'S5', 'S7');
      break;
    case 1: // SL Half Day
      $codes = array('');
      break;
    case 2: // YCH Full
      $codes = array('C%'); //array('C1', 'C4', 'C5', 'C7');
      break;
    case 3: // YCH Half
      $codes = array('');
      break;
    case 4: // Davis
      $codes = array('%D%');
      break;
    case 5: // Vaughan
      $codes = array('%V%');
      break;
    case 6: // Q
      $codes = array('%Q%');
      break;
    case 7: // Evening
      $codes = array('%E%');
      break;
    case 8: // Night Call
      $codes = array('%*%');
      break;
    case 9: // Wknd YCH
      $codes = array('Y*');
      break;
    case 10: // Wknd SL
      $codes = array('Z');
      break;
    case 11: // P
      $codes = array('%P%');
      break;
    case 12: // Business Half Day
      $codes = array('%B%');
      break;
    case 13: // IR Off Hour Callbacks
      $codes = array('');
      break;
    case 14: // Adjustments
      $codes = array('');
      break;
    case 15: // Total Worked FT
      $codes = array('');
      break;
    default:
      $codes = array('');
      break;
  }

  // Not using PDO since didn't want to waste more time trying to figure out COUNT with the GROUP BY.
  $result = db_query("SELECT COUNT(*) as nums FROM {xray_sked} WHERE uid = :uid AND code_type LIKE :rotation AND work_date >= :start AND work_date <= :end AND code LIKE (:codes)", array(':uid' => $uid, ':rotation' => 'rotation', ':start' => $start, ':end' => $end, ':codes' => $codes));

  foreach ($result as $row) {
    $num = $row->nums;
  }


  return $num;
}


/**
 * Build table with tallied shifts.
 */
function _sked_shift_tally($uid, $year, $month) {

  $max_days = date('t', strtotime($year . '-' . $month . '-01'));

  $start = $year . '-' . $month . '-' . '01';
  $end = $year . '-' . $month . '-' . $max_days; 

  // Not using PDO since didn't want to waste more time trying to figure out COUNT with the GROUP BY.
  $result = db_query("SELECT code, COUNT(*) as nums FROM {xray_sked} WHERE uid = :uid AND code_type LIKE :rotation AND work_date >= :start AND work_date <= :end GROUP BY code ORDER BY code", array(':uid' => $uid, ':rotation' => 'rotation', ':start' => $start, ':end' => $end));

  $shifts = array();
  $counter = 0;

  foreach ($result as $row) {
    $shifts[$counter]['code'] = $row->code;
    $shifts[$counter]['tally'] = $row->nums;
    $counter++;
  }

  return $shifts;

  // Was having trouble figuring out COUNT / aggregate functions with PDO.

  /*$query = db_select('xray_sked', 'x');

   $query
    ->condition('uid', $uid, '=')
    ->condition('work_date', $year . '-' . $month . '-01', '>=')
    ->condition('work_date', $year . '-' . $month . '-' . $max_days, '<=')
    ->condition('code_type', 'rotation')
    ->fields('x', array('code', 'work_date'))
 
    ->orderBy('work_date');

  $result = $query->execute();

  $shifts = array();

  foreach($result as $row) {
    $shifts[] = $row->work_date . ' - ' . $row->code;
  } */

}

/**
 * Helper function to return array of active Rads.
 */
function _sked_active_rads() {
  $query = db_select('users', 'u');
  $query->join('users_roles', 'ur', 'u.uid = ur.uid');

  $query
    ->condition('status', '1', '=')
    ->condition('rid', '4', '=')
    ->fields('u', array('uid', 'name'))
    ->orderBy('name');

  $result = $query->execute();

  $options = array();

  foreach($result as $row) {
    $options[$row->uid] = $row->name;
  }

  return $options;
}


/**
 * Returns array of shift groupings as per hours
 * worked reconciliation.
 */
function _sked_get_shift_types() {

  $types = array(
    0 => 'SL Full Day',
	1 => 'SL Half Day',
	2 => 'YCH Full Day',
	3 => 'YCH 1/2',
	4 => 'Davis',
	5 => 'Vaughan',
	6 => 'Q',
	7 => 'Evening',
	8 => 'Night Call',
	9 => 'W/E YCH',
	10 => 'W/E SL',
	11=> 'P',
	12 => 'Business 1/2 Day',
	13 => 'IR Off hour Callbacks',
	14 => 'Adjustments',
	15 => 'Total worked FT'
  );

  return $types;

}