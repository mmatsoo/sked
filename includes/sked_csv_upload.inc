<?php

/**
 * @file
 * Include file to handle csv upload / saving of schedule info into DB.
 */

// Max number of columns with useful info. Uploaded file may have more info in it, but our schedule does not care about it.
define('MAX_COLUMNS', 33);
define('RADIOLOGIST_ROLE', 4);


/**
 * Called by validate function.
 * Checks that csv has expected formatting and nothing nasty.
 * @return
 *   Boolean. True if file is bad.
 */
function _sked_csv_upload_reject_csv($form, &$form_state) {

  $file_ext = strtolower(substr($_FILES['upload']['name'],-3,3));
  $file_type = $_FILES['upload']['type'];
  $file_size = $_FILES['upload']['size'];

  // Make sure the file is too legit to quit.
  if ($file_ext !== 'csv' || ($file_type !== 'text/csv' && $file_type !== 'application/vnd.ms-excel') || (int) $file_size > 51200 ) {
    // Do not accept file.
    drupal_set_message(t('<h2>Oops! We can\'t accept the file you are uploading.</h2><p>You need to upload a <strong>.csv</strong> file. Its format has to be very specific and is based on the scheduling files used by your group.</p><p>Details for the file you have tried to upload are printed below.</p><ul><li>File extension: @file_ext</li><li>File MIME type: @file_type</li><li>File size: @file_size KB</li></ul>', array('@file_ext' => $file_ext, '@file_type' => $file_type, '@file_size' => $file_size/1024)), 'error');
    return TRUE;      
  }

  return FALSE;
  
}





/**
 * Save schedule info from CSV into DB.
 * A lot of this is taken from original site code.
 */
function _sked_csv_upload_save_sked_info($form, &$form_state) {

  // Populate arrays for matching below.
  $arrShiftCode = _get_shift_codes();
  $rads = _get_rads();

  $upload_dir = drupal_get_path('module', 'sked') . '/csv/';
  $upload_file = $upload_dir . basename($_FILES['upload']['name']);

  // $data is string with csv contents.
  $str_data = file_get_contents($_FILES['upload']['tmp_name']);

  $temp = print_r($_FILES, true);
  $temp2 = print_r($arrShiftCode, true);
  $rads_temp = print_r($rads, true);
  // watchdog('xray', $str_data . '<br /> <br />$_FILES in _submit <pre>' . $temp . '</pre> $arrShift Code <pre>' . $temp2 . '</pre> rads <pre>' . $rads_temp . '</pre>');

  // Avoid PC / Mac line-ending conflicts. Needs to be set before fopen!
  ini_set("auto_detect_line_endings", TRUE);

  $row = 1;
  $theFileName = $_FILES['upload']['name'];
  $handle = fopen($_FILES['upload']['tmp_name'], "r");

  $arr_shift_codes_nid = _get_nid_for_shift_code();

  // Instantiate object to hold miscellaneous info.
  $objInfo = new stdClass();

  $code_type_flag = 'rotation';
   
  // $data has each row of data, column by column. 
  while (($data = fgetcsv($handle, 0, ',')) !== FALSE) {

    // Have we switched to rounds or events?
    if (strtolower($data[0]) == 'rounds' || strtolower($data[0]) == 'events'):
      $code_type_flag = (strtolower($data[0]) == 'rounds') ? 'round' : 'events'; // Doing this here because in the csv template the word is actually "rounds", but our node type is called just "round". 
    endif;

//watchdog('xray', 'data <pre>' . print_r($data, true) . '</pre>');     
    // Skip first row and third rows as they do not contain info we need.
    if ($row == 1 || $row == 3):
     // continue;
    endif;

    // Month and Year info in row 2.
    if ($row == 2):
      $objInfo->month = str_pad($data[0], 2, '0', STR_PAD_LEFT);
      $objInfo->year = $data[1];
      $numDaysInMonth = cal_days_in_month(CAL_GREGORIAN, $objInfo->month, $objInfo->year);
      $objInfo->numDaysInMonth = $numDaysInMonth;

      // @TODO. Check to make sure deletion works as expected.
      $num_deleted = db_delete('xray_sked')
        ->condition('work_date', $objInfo->year . '-' . $objInfo->month . '-01', '>=')
        ->condition('work_date', $objInfo->year . '-' . $objInfo->month . '-' . $objInfo->numDaysInMonth, '<=')
        ->execute();
    endif;


     /* if ($row == 2 && $c == 0) {
        // This row contains Month info (ex. 2 == February).
        if (strlen(trim($data[$c])) < 2) {
	      $skedMonth = "0" . $data[$c];
          $objInfo->month = $skedMonth;
        } 
        else {
          $skedMonth = $data[$c];
          $objInfo->month = $skedMonth;
        }
      } 
      elseif ($row == 2 && $c == 1) {
        // This row contains Year info. (ex. 2013).
		$skedYear = $data[$c];
		$numDaysInMonth = cal_days_in_month(CAL_GREGORIAN, $skedMonth, $skedYear);

        // @TODO. Check to make sure deletion works as expected.
        $num_deleted = db_delete('xray_sked')
          ->condition('work_date', $skedYear . '-' . $skedMonth . '-01', '>=')
          ->condition('work_date', $skedYear . '-' . $skedMonth . '-' . $numDaysInMonth, '<=')
          ->execute();

      } 
      else */

      //if ($row > 3 && $c == 0) {

      // Now we are finally into actual scheduling info where each row starts with the legacy radiologist id.
      // Get Drupal user id based on legacy id.
      $drupal_user_id = _sked_csv_upload_legacy_mapping($data[0]); // $data[0] is the rad's legacy id from older system.
      if ($row > 3 && $drupal_user_id !== 0) {
//print_r($data);exit;
        // TODO. Events will be handled differently than rotations/rounds so we have to fork the logic here.

        // Getting Radiologist ID.
        $legacy_id = $data[0];
        $objInfo->legacy_id = $legacy_id;
        $objInfo->drupal_user_id = $drupal_user_id;

		$docName = $data[1];
        $objInfo->doc_name = $docName;

        reset($arrShiftCode);
		$theNumRows = count($arrShiftCode);
//print $data[34];exit;
        // Looping through columns of one row. We should only care about info in first 33 columns.
        for ($c=2; $c < MAX_COLUMNS; $c++) {
          // Only do this if we are not beyond number of days in the given month.
          if (($c-1) <= $objInfo->numDaysInMonth && isset($data[$c])):
				  // Populate variable with value from csv. Default empty value to "X".
//watchdog('xray', $data[0] . ' ' . $data[1] . ' ' . $data[$c] . ' c=' . $c);

              if (trim($data[$c]) == '') {
			    $sked_code = "X";
			  } 
			  else {
		        $sked_code = trim($data[$c]);
			  }

			  $day_for_insert = str_pad($c-1, 2, '0', STR_PAD_LEFT);

              // Check if shift code exists and grab its node id.
              // @TODO. If shift code does not exist in system, automatically create it.
              if (isset($arr_shift_codes_nid[$sked_code])) {
                $shift_code_nid = $arr_shift_codes_nid[$sked_code];
              }
              else {
                $shift_code_nid = 36; // nid = 36 is a rotation node dev made as a catch-all for unknown shift codes.
              }

              // Trying to fix annoying timestamp problem where timezone and offsets seem to make shift codes on first of month not show up.
              if ($day_for_insert === '01') {
                $time_stamp = strtotime($objInfo->year . '-' . $objInfo->month . '-' . $day_for_insert) + 36000;
              }
              else {
                $time_stamp = strtotime($objInfo->year . '-' . $objInfo->month . '-' . $day_for_insert);
              }

			  $sid = db_insert('xray_sked')
					->fields(array(
					  'uid' => $objInfo->drupal_user_id,
					  'nid' => $shift_code_nid,
					  'rad_id' => $objInfo->legacy_id,
					  'code' => $sked_code,
                      'code_type' => $code_type_flag,
					  'call_flag' => $call = (strpos($sked_code, '*') !== FALSE) ? 1 : 0,
					  'work_date' => $objInfo->year . '-' . $objInfo->month . '-' . $day_for_insert,
					  'work_date_ts' => $time_stamp,
			   ))
			   ->execute();

          endif;

        }

      }

    $row++;
  }

  fclose($handle);

  // Perform duplicate and missing shift checks.
  $objDuplicate = sked_duplicate_shift_check($objInfo->year . '-' . $objInfo->month, 'month');
  $objMissing = sked_missing_shift_check($objInfo->year . '-' . $objInfo->month, 'month');

  

 //drupal_set_message(t('Schedule uploaded for @month @year.', array('@month' => date("F", mktime(0, 0, 0, $objInfo->month, 10)), '@year' => $objInfo->year)) . '<br /><br /><ul><li><a href=href="/admin/config/sked/missing/' . $objInfo->year . '-' . $objInfo->month . '/month">' . t('Check for missing shifts in uploaded month.') . '</a> (hint: Open this link in new browser tab. Right click and choose "Open (link) in new tab")</li><li><a href="/admin/config/sked/duplicate/' . $objInfo->year . '-' . $objInfo->month . '/month">' . t('Check for duplicate shifts in uploaded month.') . '</a> (hint: Open this link in new browser tab. Right click and choose "Open (link) in new tab")</li></ul>');

  drupal_set_message(t('Schedule uploaded for @month @year.', array('@month' => date("F", mktime(0, 0, 0, $objInfo->month, 10)), '@year' => $objInfo->year)));
  drupal_set_message($objDuplicate->strOutput, $objDuplicate->status);
  drupal_set_message($objMissing->strOutput, $objMissing->status);


			/*  if ($c < 10) { // DAYS BELOW 10 WILL HAVE A ZERO ADDED FOR INSERT.
				  for($x=1;$x < $theNumRows;$x++) {
					if (strtoupper($skedCode) == strtoupper($arrShiftCode[$x]['shift_code'])) {
					  $skedCodeID = $arrShiftCode[$x]['nid'];

					  $sid = db_insert('xray_sked')
						->fields(array(
						  'uid' => $drupal_user_id,
						  'rad_id' => $docID,
						  'rotation' => $skedCode,
						  'call_flag' => $call = (strpos($skedCode, '*') !== FALSE) ? 1 : 0,
						  'work_date' => $skedYear . '-' . $skedMonth . '-0' . $c,
						  'work_date_ts' => strtotime($skedYear . '-' . $skedMonth . '-0' . $c),
						))
						->execute();
					}
				  }
			  } 
			  else {
				for($x=1;$x < $theNumRows;$x++) {
				  if (strtoupper($skedCode) == strtoupper($arrShiftCode[$x]['shift_code'])) {
					$skedCodeID = $arrShiftCode[$x]['nid'];

					  $sid = db_insert('xray_sked')
						->fields(array(
						  'uid' => $drupal_user_id,
						  'rad_id' => $docID,
						  'rotation' => $skedCode,
						  'call_flag' => $call = (strpos($skedCode, '*') !== FALSE) ? 1 : 0,
						  'work_date' => $skedYear . '-' . $skedMonth . '-' . $c,
						  'work_date_ts' => strtotime($skedYear . '-' . $skedMonth . '-' . $c),
						))
						->execute();
				  }
				}
			  } */



	  //} 
      //elseif ($row > 3 && $c == 1) {
        // Now we are finally into first day of month which means the actual day and shift code info.
		/*$docName = $data[$c];
		if (trim($data[$c+1]) == "") { //c+1 since the sked code for first day is in next column
		  $skedCode = "X";
		} 
        else {
		  $skedCode = trim($data[$c+1]);
		}*/

/*
          reset($arrShiftCode);
		  $theNumRows = count($arrShiftCode);

		  for($x=1;$x < $theNumRows;$x++) {
		    if (strtoupper($skedCode) == strtoupper($arrShiftCode[$x]['shift_code'])) {
		      $skedCodeID = $arrShiftCode[$x]['nid'];

              $sid = db_insert('xray_sked')
                ->fields(array(
                  'uid' => $drupal_user_id,
                  'rad_id' => $docID,
                  'rotation' => $skedCode,
                  'call_flag' => $call = (strpos($skedCode, '*') !== FALSE) ? 1 : 0,
                  'work_date' => $skedYear . '-' . $skedMonth . '-0' . $c,
                  'work_date_ts' => strtotime($skedYear . '-' . $skedMonth . '-0' . $c),
                ))
                ->execute();
		    }
	      }
		$theDay = 2;
	  } 
*/
/*
      elseif (($row > 3) && ($c >= 2) && ($c <= $numDaysInMonth)) {
	    //echo $data[$c] . ",";
		if (trim($data[$c+1]) == "") {
		  $skedCode = "X";
		} 
        else {
		  $skedCode = trim($data[$c+1]);
		}

*/ 
/*     if ($c < 10) { //DAYS BELOW 10 WILL HAVE A ZERO ADDED FOR INSERT
		  reset($arrShiftCode);
		  $theNumRows = count($arrShiftCode);
		  for($x=1;$x < $theNumRows;$x++) {
		    if (strtoupper($skedCode) == strtoupper($arrShiftCode[$x]['shift_code'])) {
			  $skedCodeID = $arrShiftCode[$x]['nid'];

              $sid = db_insert('xray_sked')
                ->fields(array(
                  'uid' => $drupal_user_id,
                  'rad_id' => $docID,
                  'rotation' => $skedCode,
                  'call_flag' => $call = (strpos($skedCode, '*') !== FALSE) ? 1 : 0,
                  'work_date' => $skedYear . '-' . $skedMonth . '-0' . $c,
                  'work_date_ts' => strtotime($skedYear . '-' . $skedMonth . '-0' . $c),
                ))
                ->execute();
			}
		  }
	  } 
      else {
	    reset($arrShiftCode);
		$theNumRows = count($arrShiftCode);
		for($x=1;$x < $theNumRows;$x++) {
		  if (strtoupper($skedCode) == strtoupper($arrShiftCode[$x]['shift_code'])) {
		    $skedCodeID = $arrShiftCode[$x]['nid'];

              $sid = db_insert('xray_sked')
                ->fields(array(
                  'uid' => $drupal_user_id,
                  'rad_id' => $docID,
                  'rotation' => $skedCode,
                  'call_flag' => $call = (strpos($skedCode, '*') !== FALSE) ? 1 : 0,
                  'work_date' => $skedYear . '-' . $skedMonth . '-' . $c,
                  'work_date_ts' => strtotime($skedYear . '-' . $skedMonth . '-' . $c),
                ))
                ->execute();
	      }
	    }
      } */

      //$theDay++;

      //}




}


/**
 * Get shift codes and their node ids.
 * @return
 *   Array. Associative array.
 */
function _get_nid_for_shift_code() {
  $type = 'rotation'; 
  $nodes = node_load_multiple(array(), array('type' => $type)); 

  $arr = array();

  // Ex. $arr['C3'] = 13
  foreach($nodes as $rotation) {
    $arr[$rotation->field_shift_code['und'][0]['value']] = $rotation->nid;
  }

  return $arr;

}


/**
 * Map legacy radiologist id to drupal user id.
 * @param
 *   Integer. Legacy rad id.
 * @return
 *   Integer. Drupal user id.
 */
function _sked_csv_upload_legacy_mapping($docID) {

  $drupal_user_id = db_query("SELECT entity_id FROM {field_data_field_legacy_radiologist_id} WHERE field_legacy_radiologist_id_value = :legacy_id", array(':legacy_id' => $docID))->fetchField();

  if (gettype($drupal_user_id) == 'string') {
    return $drupal_user_id;
  }
  else {
    return 0;
  }
}


/**
 * Helper function to return array with shift codes.
 */
function _get_shift_codes() {
  // Get Shift Codes.
  $result = db_query("SELECT nid FROM {node} WHERE type = :type", array(':type' => 'rotation'));

  // Ready to populate array with shift code info. This is approach from original site.
  $arrShiftCode = array();
  $rowCounter = 1;

  foreach($result as $row) {
    $node = node_load($row->nid);

    $arrShiftCode[$rowCounter]['nid'] = $node->nid;
    $arrShiftCode[$rowCounter]['shift_code'] = $node->field_shift_code['und'][0]['value'];
    $rowCounter++;
  }

  return $arrShiftCode;
}


/**
 * Helper function to return array with radiologist user ids as per radiologist role.
 */
function _get_rads() {
  // Get radiologist info.
  $result = db_query("SELECT uid FROM {users_roles} WHERE rid = :role", array(':role' => RADIOLOGIST_ROLE));

  // Get info about radiologists to match incoming docID with new ID.
  $rads = array();
  $rowCounter = 0;

  foreach($result as $row) {
    $user = user_load($row->uid);

    $rads[$rowCounter]['uid'] = $user->uid;
    $rads[$rowCounter]['legacy_rad_id'] = $user->field_legacy_radiologist_id['und'][0]['value'];
    $rowCounter++;
  }

  return $rads;
}