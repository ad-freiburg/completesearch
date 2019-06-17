<?php
/*
 * Set the text member of the AC object which contains all text pattern for the supported lanuages of the user interface.
 */

	// With this function you can provide an own style for error message
	function error_style ($text) { return "[$text]"; }


	$AC->text["javascript_note_title"] = array
	(
		"de" => "Hinweis zu Javascript",
		"en" => "Note to Javascript"
	);


	$AC->text["javascript_note_body"] = array
	(
		"de" => "Sie haben Javascript ausgeschaltet. Die Auto-Vervollständigung (ein wesentliches Merkmal unserer Suche) erhalten Sie nur, falls Sie Javascript erlauben",
		"en" => "Javascript is disabled. To search as you type - a main feature of our search - you need to enable javascript!"
	);

  $AC->text["javascript_note"] = $AC->text["javascript_note_body"];

  $AC->text["cookie_note"] = array
  (
    "de" => "Ihr Browser lässt keine Cookies zu und wir können keinen vollen Funktionsumfang garantieren. Bei Problemen schreiben Sie eine Mail.",
    "en" => "Cookies are disabled and we can't garantuee full functionality. In case of problems, just write an email."
  );

	$AC->text["note_hint"] = array
	(
		"de" => "Hinweis, klicken Sie hier für weitere Informationen",
		"en" => "Note, click here to get more information"
	);

	$AC->text["title"] = array
	(
		"de" => "Suchen mit Auto-Vervollständigung",
		"en" => "Searching with Autocompletion"
	);

	$AC->text["subtitle"] = array
	(
		"de" => "herangezoomt auf %s Dokumente",
		"en" => "zoomed in on %s documents"
	);

	$AC->text["subtitle_idle"] = array
	(
		"de" => "Suche über %s Dokumente",
		"en" => "searching in %s documents"
	);

	$AC->text["subtitle_no_hits"] = array
	(
		"de" => "Keine Treffer",
		"en" => "No hits"
	);

	$AC->text["default_page"] = array
	(
		"de" => "Default Seite",
		"en" => "Default page"
	);

	$AC->text["completions_result"] = array
	(
		"de" => "Verfeinere nach WORT",
		"en" => "Refine by WORD",
		//"de" => "Verfeinere nach WORT (%d)",
		//"en" => "Refine by WORD (%d)",
		//"de" => "%s Vervollständigungen von \"%s\" führen zu einem Treffer:",
		//"en" => "%s completions of \"%s\" lead to a hit:"
	);

	$AC->text["completions_result_Y"] = array
	(
		"de" => "Verfeinere nach %s (%d)",
		"en" => "Refine by %s (%d)",
	);

	$AC->text["completions_result_R"] = array
	(
		"de" => "Verfeinere nach %s (%d)",
		"en" => "Refine by %s (%d)",
	);

	$AC->text["completions_title_on_error"] = array
	(
		"de" => "Ein Fehler ist aufgetreten",
		"en" => "An error occurred"
		//"de" => "Vervollständigungen:",
		//"en" => "Completions:"
	);

	$AC->text["completions_result_singular"] = array
	(
                "de" => $AC->text["completions_result"]["de"],
                "en" => $AC->text["completions_result"]["en"],
		//"de" => "Eine Vervollständigung von \"%s\" führt zu einem Treffer:",
		//"en" => "One completion of \"%s\" leads to a hit:"
	);

	$AC->text["completions_no_result"] = array
	(
                "de" => $AC->text["completions_result"]["de"],
                "en" => $AC->text["completions_result"]["en"],
		//"de" => "Keine Vervollständigung von \"%s\" führt zu einem Treffer!",
		//"en" => "No completion of \"%s\" leads to a hit!"
	);

	$AC->text["completions_larger"] = array
	(
                "de" => $AC->text["completions_result"]["de"],
                "en" => $AC->text["completions_result"]["en"],
		//"de" => "%s Vervollständigungen von \"%s\" führen zu einem Treffer:",
		//"en" => "%s completions of \"%s\" lead to a hit:"
	);

	$AC->text["completions_too_large"] = array
	(
                "de" => $AC->text["completions_result"]["de"],
                "en" => $AC->text["completions_result"]["en"],
		//"de" => "Mehr als %s Vervollständigung von \"%s\" führen zum einem Treffer, formulieren Sie Ihre Anfrage bitte genauer!",
		//"en" => "More than %s completions of \"%s\" lead to a hit, type more!"
	);

	$AC->text["completion_title_empty_query"] = array
	(
		"de" => "", //"** leere Suchanfrage **",
		"en" => "" //"** empty query **"
	);

	$AC->text["completion_box_empty_query"] = array
	(
		"de" => "",
		"en" => ""
		//"de" => "Keine Vervollständigungen (wegen leerer Anfrage)",
		//"en" => "No completions because of empty query"
	);

	$AC->text["completion_title_at_beginning"] = array
	(
		"de" => "",
		"en" => "",
                //"de" => "** Start **",
		//"en" => "** empty query **"
	);

	$AC->text["completion_box_at_beginning"] = array
	(
	        "de" => $AC->text["completion_box_empty_query"]["de"],
	        "en" => $AC->text["completion_box_empty_query"]["en"],
		//"de" => "Keine Vervollständigungen (wegen leerer Anfrage)",
		//"en" => "No completions because of empty query"
	);

    // NEW 6Aug06 (Holger): now distinguished text elements for cn: completions

    $AC->text["cn_completions_result"] = array
    (
        "de" => "%s Kategorien mit \"%s\" führen zu einem Treffer:",
        "en" => "%s categories matching \"%s\" lead to a hit:"
    );

    $AC->text["cn_completions_result_singular"] = array
    (
        "de" => "Eine Kategorie mit \"%s\" führt zu einem Treffer:",
        "en" => "One category matching \"%s\" leads to a hit:"
    );

    $AC->text["cn_completions_no_result"] = array
    (
        "de" => "Keine Kategorie mit \"%s\" führt zu einem Treffer!",
        "en" => "No category matching \"%s\" leads to a hit!"
    );

    $AC->text["cn_completions_larger"] = array
    (
        "de" => "%s Kategorien mit \"%s\" führen zu einem Treffer:",
        "en" => "%s categories matching \"%s\" lead to a hit:"
    );

    $AC->text["cn_completions_too_large"] = array
    (
        "de" => "Mehr als %s Kategorien mit \"%s\" führen zu einem Treffer:",
        "en" => "More than %s categories matching \"%s\" lead to a hit:"
    );

    $AC->text["facets_title_on_error"] = array
    (
      "de" => $AC->text["completions_title_on_error"]["de"],
      "en" => $AC->text["completions_title_on_error"]["en"],
      //"de" => "Verfeinerung:",
      //"en" => "Refinements:"
    );

    $AC->text["refine_by"] = array
    (
        "de" => "Verfeinere nach %s",
        "en" => "Refine by %s"
    );

	$AC->text["hits_title_no_hits"] = array
	(
		"de" => "Keine Treffer",
		"en" => "No hits"
	);

	$AC->text["hits_title_one_hit"] = array
	(
		"de" => "Ein einzelner Treffer",
		"en" => "A single hit"
	);

	$AC->text["hits_title"] = array
	(
		// NEW 03.08.06 (Markus): query string added
		// NEW 07.08.06 (Markus): css class for images added
		"de" => "Treffer <b>%s</b> - <b>%s</b> von <b>%s</b> für <b>%s</b> (%sBild <img class=\"arrow_img\" src='%s' border='0'>%s / %sBild <img class=\"arrow_img\" src='%s' border='0'>%s für die nächsten/vorherigen Treffer)",
		"en" => "Hits <b>%s</b> - <b>%s</b> of <b>%s</b> for <b>%s</b> (%sPageUp <img class=\"arrow_img\" src='%s' border='0'>%s / %sPageDown <img class=\"arrow_img\" src='%s' border='0'>%s for next/previous hits)"
	);

	$AC->text["hit_box_at_beginning"] = array
	(
		"de" => "",
		"en" => "",
		//"de" => "<img src='images/intro.gif'><br><br><p id=\"hit_box_at_beginning\">Suche über %s Dokumente der Kollektion %s (%s).<br>Geben Sie bitte Ihre Anfrage ein!</p>",
		//"en" => "<img src='images/intro.gif'><br><br><p id=\"hit_box_at_beginning\">Searching over %s documents of the collection %s (%s).<br>Type in your query, please!</p>"
	);

	$AC->text["hit_box_empty_query"] = array
	(
		"de" => "",
		"en" => ""
		//"de" => "<p id=\"hit_box_at_beginning\">Geben Sie bitte Ihre Anfrage ein!</p>",
		//"en" => "<p id=\"hit_box_at_beginning\">Type in your query, please!</p>"
	);

	$AC->text["feedback"] = array
	(
		"de" => "Verwenden Sie bitte das Textfeld unten, um uns Feedback zu geben",
		"en" => "Give us feedback in the textarea below, please"
	);

	$AC->text["help_title"] = array
	(
		"de" => "Ein wenig Hilfe",
		"en" => "A little help"
	);

	$AC->text["help"] = array
	(
		"de" => "",
		"en" =>
		"<center><p>While you type, completions of what you have started typing will be displayed,
		as well as hits for any of these completions.</p>
		<p>Type a dot . between two words, if they should occur right next to each other,
		<br>two dots .. if they should occur within 5 words of each other,
		<br>three dots ... if they should occur within 15 words of each other.
		<br>End a word with a dollar $ if you want only the exact words (no completions) to match.
		<br>A vertical bar | between two words means searching for one word OR the other.</p>
		<p>
		Typing ? at any point in the query field will provide a short explanation.
		</p></center>"
	);

	$AC->text["send"] = array
	(
		"de" => "Absenden",
		"en" => "Send this"
	);

	$AC->text["copyright"] = array
	(
//		"de" => "<p>Geben Sie ? an beliebiger Stelle im Eingabfeld ein, um eine kurze Hilfe zu bekommen.</p>&copy; MPII AGI-IR",
//		"en" => "<p>Typing ? at any point in the query field will provide a short explanation.</p>&copy; MPII AGI-IR"
		"de" => "Geben Sie Ihre Suchbegriffe im Eingabefeld ein.<br>&copy; MPII AGI-IR",
		"en" => "Type in your search query.<br>&copy; MPII AGI-IR"
	);

	$AC->text["options_title"] = array
	(
		"de" => "Einstellungen",
		"en" => "Options"
	);

	$AC->text["options_subtitle"] = array
	(
		"de" => "Hier können Sie verschiedene Einstellungen verändern.",
		"en" => "Here you can change your preferences."
	);

//	$AC->text["options_notes"] = array
//	(
//		"de" => "Falls Sie Cookies erlauben, bleiben Ihre Änderungen auch für zukünftige Sitzungen erhalten.",
//		"en" => "If you have cookies allowed, your changes will persist for future sessions."
//	);

	$AC->text["options_notes_cookies_allowed"] = array
	(
		"de" => "Ihre Änderungen werden als Cookie gespeichert und bleiben für zukünftige Sitzungen erhalten.",
		"en" => "Your changes are saved as a cookie and will persist for future sessions."
	);

	$AC->text["options_notes_cookies_not_allowed"] = array
	(
		"de" => "Sie haben Cookies abgeschaltet. Falls Sie Cookies erlauben, bleiben Ihre Änderungen auch für zukünftige Sitzungen erhalten.",
		"en" => "You have cookies disabled. Allow cookies to make your changes permanent.",
	);

	// TEXTS FOR THE OPTIONS PAGE (OPTIONS.PHP)
	//
	$AC->text["max_completions_compute"] = array
	(
		"de" => "Wieviele Vervollständigungen sollen maximal berechnet werden?",
		"en" => "How many completions should be computed at most?"
	);

	$AC->text["max_completions_show"] = array
	(
		"de" => "Wieviele Vervollständigungen sollen maximal angezeigt werden?",
		"en" => "How many completions should be shown at most?"
	);

	$AC->text["max_completions_show_right"] = array
	(
		"de" => "Wieviele Vervollständigungen sollen maximal angezeigt werden, wenn sich eine Box im rechten Detailbereich befindet?",
		"en" => "How many completions should be shown at most if a box is located in the right detail area?"
	);

	$AC->text["max_completion_length"] = array
	(
		"de" => "Mit welcher maximalen Zeichenlänge sollen Vervollständigungen angezeigt werden?<br>(der rest wird abgeschnitten und als '...' angezeigt)",
		"en" => "With which maximum length completions should be shown<br>(rest is truncated and shown as '...')?"
	);

	$AC->text["hits_per_page_while_typing"] = array
	(
		"de" => "Wieviele Treffer sollen während des Tippens pro Seite angezeigt werden?",
		"en" => "How many hits per page should be shown while typing?"
	);
        
        $AC->text["hits_per_page_on_click"] = array
	(
		"de" => "Wieviele Treffer sollen bei einem Klick pro Seite angezeigt werden?",
		"en" => "How many hits per page should be shown on click?"
        );

	$AC->text["excerpts_per_hit"] = array
	(
		"de" => "Wieviele Textauszüge sollen pro Treffer angezeigt werden?",
		"en" => "How many excerpts per hit should be shown?"
	);

	$AC->text["excerpt_radius"] = array
	(
		"de" => "Wieviele Worte sollen rechts und links der gefundenen Treffer angezeigt werden?",
		"en" => "How many words should be shown to the left and right of each match?",
	);

	$AC->text["display_mode"] = array
	(
		"de" => "Welcher Anzeige-Modus soll verwendet werden?",
    "en" => "Which display mode should be used?<br/>(1 = normal, 2 = each match in its own line, 3 = no urls)",
	);

	$AC->text["synonym_mode"] = array
	(
		"de" => "Which synonym mode should be used?<br>(0 = never expand, 1 = expand when ~ at end, 2 = expand all)",
		"en" => "Which synonym mode should be used?<br>(0 = never expand, 1 = expand when ~ at end, 2 = expand all)"
	);

	$AC->text["debug_mode"] = array
	(
		"de" => "Which debug mode should be used?<br>(0 = show completions nicely, 1 = show in raw form)",
		"en" => "Which debug mode should be used?<br>(0 = show completions nicely, 1 = show in raw form)"
        );

	$AC->text["log_level"] = array
	(
		"de" => "Welcher Level beim Logging soll verwendet werden?<br>(1 = Fatal, 2 = Fehler, 3 = Warnung, 4 = Info, 5 = Debug)",
		"en" => "Which log level should be used?<br>(1 = fatal, 2 = error, 3 = warning, 4 = info, 5 = debug)"
	);

	$AC->text["query_types"] = array
	(
    "de" => "Welche Arten von Ergebnissen sollen angezeigt werden?<br/>(W = Worte, H = Treffer, F = Facetten, C = Kategorien)",
		"en" => "Which kinds of results should be shown?<br/>(W = words, H = hits, F = facets, C = categories)"
	);

	$AC->text["more_offset"] = array
	(
		"de" => "Um wieviel soll die Anzahl der Boxen-Einträge beim Klick auf die 'mehr'/'weniger'-Links verändert werden?",
		"en" => "How should the amount of completions be changed when cklicked on the 'more'/'less' links?"
	);

	$AC->text["append_to_clicked_completion"] = array
	(
		"de" => "Welche Zeichenkette soll an eine angeklickte Vervollständigung angehängt werden?",
		"en" => "What string to append when clicking a completion?"
	);

	$AC->text["append_to_clicked_cat_completion"] = array
	(
		"de" => "Welche Zeichenkette soll an ct/cn-Vervollständigungen angehängt werden?",
		"en" => "What string to append to ct/cn completions?"
	);

	$AC->text["button_back"] = array
	(
		"de" => "Zurück",
		"en" => "Back"
	);

	$AC->text["button_apply"] = array
	(
		"de" => "Übernehmen",
		"en" => "Apply"
	);

	$AC->text["button_restore_defaults"] = array
	(
		"de" => "Standardwerte wiederherstellen",
		"en" => "Restore default values"
	);

	$AC->text["options_changed_title"] = array
	(
		"de" => "Einstellungen geändert",
		"en" => "Options changed"
	);

	$AC->text["options_changed_note"] = array
	(
		"de" => "Die Einstellungen wurden geändert. Klicken Sie auf die Schaltfläche rechts, um zur Startseite zurückzukehren",
		"en" => "The options were changed. Please click on the button to the right to go back to the start page"
	);

//	$AC->text["options_changed"] = array
//	(
//		"de" => "Die Einstellungen wurden geändert",
//		"en" => "Options changed"
//	);
//
	$AC->text["options_changed_and_go_back"] = array
	(
		"de" => "Die Einstellungen wurden geändert, ich gehe zur Suche zurück ...",
		"en" => "Options changed, I'm going back to search window ..."
	);

	$AC->text["socket_connected"] = array
	(
		"de" => "Verbindung zum Completion-Server über Socket auf Port %s hergestellt",
		"en" => "To completion server via socket on port %s established"
	);

	$AC->text["query_too_short"] = array
	(
		"de" => "Geben Sie bitte mindestens %s Zeichen ein",
		"en" => "Type at least %s characters per word"
	);

	$AC->text["error_no_server_connection"] = array
	(
		"de" => error_style("Zeitüberschreitung bei Anfrage"),
		"en" => error_style("Timeout for query")
//		"de" => error_style("Zur Zeit ist keine Verbindung zum Server möglich"),
//		"en" => error_style("There is no server connection at this moment")
	);

	// Text for subtitle when we get an connection timeout
	$AC->text["subtitle_on_no_server_connection"] = array
	(
		"de" => error_style("Zeitüberschreitung bei Anfrage"),
		"en" => error_style("Timeout for query")
	);

	$AC->text["error_open_socket"] = array
	(
		"de" => error_style("Verbindung zum Server nicht möglich (Socketfehler)"),
		"en" => error_style("Could not connect to server (socket error)")
	);

	// Text for subtitle when we can not connect to server
	$AC->text["subtitle_on_error_open_socket"] = array
	(
		"de" => error_style("Verbindung zum Server nicht möglich"),
		"en" => error_style("Could not connect to server")
	);

	$AC->text["error_server_connection"] = array
	(
		"de" => error_style("Fehler bei Kommunikation mit Server"),
		"en" => error_style("Error while communicating with server")
	);

	$AC->text["subtitle_on_error_server_connection"] = array
	(
		"de" => error_style("Fehler bei Kommunikation mit Server"),
		"en" => error_style("Error while communicating with server")
	);

//	$AC->text["error_socket_timeout"] = array
//	(
//		"de" => "Ein Verbindungsversuch per Socket auf Port %s ist fehlgeschlagen (10060). Ich versuche eine Verbindung über HTTP-Protokoll ...",
//		"en" => "Timeout while connecting via socket on port %s to server (10060). I try to connect via http ..."
//	);

	$AC->text["error_text_missing"] = array
	(
		"de" => "Zur ID '%s' existiert kein Textbaustein",
		"en" => "There is no text pattern for ID '%s'"
	);

	$AC->text["error_message_title"] = array
	(
		"de" => error_style("Es sind Fehler aufgetreten"),
		"en" => error_style("There are errors")
	);

	$AC->text["session_expired_and_recreated"] = array
	(
		"de" => "Die Sitzung war abgelaufen; sie wurde neu erzeugt und mit den Startwerten initialisiert",
		"en" => "The session was expired, she is recreated and initialized with start values"
	);

	// Problematic because AC did not exist when this error occurs ;-)
	$AC->text["error_page_directly_called"] = array
	(
		"de" => "Das Objekt AC ist nicht verfügbar. Wahrscheinlich haben Sie diese Seite direkt aufgerufen. Bitte starten Sie die Anwendung über die Hauptseite",
		"en" => "No AC available. Probably you called this page directly (standalone). Please start main page first!"
	);

	$AC->text["more"] = array
	(
		"de" => "mehr",
		"en" => "more"
	);

	$AC->text["less"] = array
	(
		"de" => "weniger",
		"en" => "less"
	);


	$AC->text["all"] = array
	(
		"de" => "alle",
		"en" => "all"
	);


	$AC->text["top"] = array
	(
		"de" => "Top",
		"en" => "top"
	);


	$AC->text["search_button"] = array
	(
		"de" => "suchen",
		"en" => "search"
	);





	/*
		Text patterns for javascript
	*/
	$AC->js_text["info_searching"] = array
	(
		"de" => "Ich suche nach '%s'",
		"en" => "I'am looking for '%s'"
	);

	$AC->js_text["button_unset_session"] = array
	(
		"de" => "Anfrage zurücksetzen",
		"en" => "Reset query"
	);

	$AC->js_text["searching ..."] = array
	(
		"de" => "Suche ...",
		"en" => "Searching ..."
	);

	$AC->js_text["quirks_mode_note_title"] = array
	(
		"de" => "Quirks-Modus",
		"en" => "Quirks mode"
  );
  $AC->js_text["quirks_mode_note_body"] = array
	(
		"de" => "Die Anwendung läuft nicht im Standard-Modus (sondern im Quirks-Modus); das Layout wird möglicherweise nicht korrekt dargestellt.<br><br>Es handelt sich sehr wahrscheinlich um einen Fehler in der Anwendung. Bitte informieren Sie uns per E-Mail an folgende Adresse: <a href=\"mailto:Holger Bast <bast@mpi-inf.mpg.de>?subject=Quirks mode\">bast@mpi-inf.mpg.de</a>.",
		"en" => "Application don't run in standard mode (but in quirks mode); layout may be displayed uncorrectly.<br><br>Probably this is an error. Please be so kind to inform us about that via e-mail to: <a href=\"mailto:Holger Bast <bast@mpi-inf.mpg.de>?subject=Quirks mode\">bast@mpi-inf.mpg.de</a>."
  );

	/*
		The following patterns are declared as empty string, means take them from $text array
	*/
	$AC->js_text["query_too_short"] = "";
	$AC->js_text["error_no_server_connection"] = "";
	$AC->js_text["error_text_missing"] = "";
	$AC->js_text["note_hint"] = "";
	// NEW 31.10. (Markus)
//	$AC->js_text["error_box_title"] = "";
?>
