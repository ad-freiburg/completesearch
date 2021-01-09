<?php

//echo "lade xml...";
class xml
{   
    var  $xml_pfad = false;
    var  $xml_stream = false;
    
    var  $arrOutput = array();
    var  $resParser = false;
    var  $strXmlData= false;

// =================================================================================================
// KONSTRUKTOR
// =================================================================================================
    function xml ( $xml_string )
    {
        $this->xml_stream = $xml_string; 
    }
   
// =================================================================================================
// LADE AUS EINER DATEI
// =================================================================================================
    function load ( $xml_pfad )
    {
        if ( file_exists( $xml_pfad ) )
        {
            $this->xml_pfad = $xml_pfad;
            $this->xml_stream = file_get_contents( $xml_pfad );
        }
        else
        {
            echo $xml_pfad." nicht gefunden!";
        }
    }

// =================================================================================================
// RÜCKGABE DES XML ARRAY
// =================================================================================================
    function get_xml_array()
    { 
        return $this->parse( $this->xml_stream );
    }
    
// =================================================================================================
// XML PARSER
// =================================================================================================
    function parse( $strInputXML )
    {   
        $this->resParser = xml_parser_create ();
        xml_set_object( $this->resParser,$this );
        xml_set_element_handler( $this->resParser, "tagOpen", "tagClosed" );
        
        xml_set_character_data_handler( $this->resParser, "tagData" );
        
        $this->strXmlData = xml_parse( $this->resParser, $strInputXML );
        
        if( !$this->strXmlData )
        {
          // Markus / 01-07-09: 1 means that an error occurred
          $error_msg = sprintf("XML error: %s at line %d",
                        xml_error_string(xml_get_error_code( $this->resParser ) ),
                        xml_get_current_line_number( $this->resParser ) );
          xml_parser_free( $this->resParser );
            
          return array(1, $error_msg );
        }

        // Markus / 01-07-09: 0 means no error
        xml_parser_free( $this->resParser );
        return array(0, $this->arrOutput);
    }
// =================================================================================================
// TAG OPEN
// =================================================================================================
    function tagOpen( $parser, $name, $attrs )
    {
       $tag=array( "name"=>$name, "attrs"=>$attrs );
       array_push( $this->arrOutput, $tag );
    }
// =================================================================================================
// TAG DATA
// =================================================================================================
    function tagData( $parser, $tagData )
    {
       if( trim( $tagData ) )
       {
            if( isset( $this->arrOutput[ count( $this->arrOutput )-1 ]['tagData']))
            {
                $this->arrOutput[ count( $this->arrOutput )-1 ]['tagData'] .= $tagData;
            }
            else
            {
                $this->arrOutput[ count( $this->arrOutput )-1 ]['tagData'] = $tagData;
            }
       }
    }
// =================================================================================================
// TAG CLOSED
// =================================================================================================
    function tagClosed( $parser, $name )
    {
       $this->arrOutput[ count($this->arrOutput )-2 ]['children'][] = $this->arrOutput[ count( $this->arrOutput )-1 ];
       array_pop( $this->arrOutput );
    }
}
//echo "geladen xml...";

?>