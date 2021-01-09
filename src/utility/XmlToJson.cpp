// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Ina Baumgarten <baumgari>.

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <utility>
#include <stack>
#include <vector>
#include "server/Exception.h"
#include "./XmlToJson.h"

using namespace std; // NOLINT

// ____________________________________________________________________________
XmlToJson::XmlToJson(const vector<string>& multiples)
{
  for (size_t i = 0; i < multiples.size(); i++)
    _multiples[multiples[i]] = true;
}

// ____________________________________________________________________________
string XmlToJson::xmlToJson(const string& xml)
{
  const string& xmlCopy = maskXml(xml);
  const Node& root = getXmlTreeIterative(xmlCopy);
  string json = convertNodeToJson(root);
  unmaskJson(&json);
  return json;
}

// ____________________________________________________________________________
string XmlToJson::maskXml(const string& xml)
{
  // It's easier for parsing, if we just mask "dangerous" characters like quotes
  // and backslahes and unmask them at the end.
  // CDATA parts should not be parsed. To make it simple just cut them out at
  // the beginning and replace them by some unique code and insert them at
  // the end. But escape it!

  _replacements.clear();
  _replacementOffsets.clear();

  string maskedXml;
  maskedXml.reserve(xml.size());

  size_t offset = 0;
  for (size_t i = 0; i < xml.size(); i++)
  {
    // Mask escaped backslash or quote.
    if (xml[i] != '<' && xml[i] != '\\' && xml[i] != _replacementChar)
      maskedXml.append(1, xml[i]);
    else if ((i <= xml.size() - 2)
        && (xml[i] == '\\'
        && (xml[i + 1] == '\"' || xml[i + 1] == '\\')))
    {
      _replacements.append(xml.substr(i++, 2));
      _replacementOffsets.push_back(offset);
      offset += 2;
      maskedXml.append(1, _replacementChar);
    }
    else if ((i <= xml.size() - 12) && xml.compare(i, 9, "<![CDATA[") == 0)
    {
      i += 9;
      size_t cdataEnd = xml.find("]]>", i);
      _replacementOffsets.push_back(offset);
      offset += cdataEnd - i;
      _replacements.reserve(_replacements.size() + cdataEnd - i);
      // Escape cdata string, but take care to not escape already escaped
      // letters.
      while (i < cdataEnd)
      {
        if (xml[i - 1] != '\\' && xml[i] == '\"')
          _replacements.append(1, '\\');
        if (xml[i - 1] != '\\' && xml[i] == '\\'
            && xml[i + 1] != '\\' && xml[i + 1] != '\"')
          _replacements.append(1, '\\');
        _replacements.append(1, xml[i]);
        i++;
      }
      i += 2;
      maskedXml.append(1, _replacementChar);
    }
    else if (xml[i] == _replacementChar)
    {
      _replacements.append(1, _replacementChar);
      _replacementOffsets.push_back(offset++);
      maskedXml.append(1, _replacementChar);
    }
    else
    {
      maskedXml.append(1, xml[i]);
    }
  }
  maskedXml.shrink_to_fit();
  return maskedXml;
}

// ____________________________________________________________________________
void XmlToJson::unmaskJson(string* json) const
{
  json->reserve(json->size() + _replacements.size()
                 - _replacementOffsets.size());
  // Replace cdata codes by cdata parts.
  int k = _replacementOffsets.size() - 1;
  size_t end = string::npos;
  for (size_t i = json->size() - 1; i != 0; i--)
  {
    if (json->at(i) == _replacementChar)
    {
      CS_ASSERT_GT(k, -1);
      json->replace(i, 1, _replacements.substr(_replacementOffsets[k],
                                               end - _replacementOffsets[k]));
      end = _replacementOffsets[k--];
    }
  }
}


// ____________________________________________________________________________
string XmlToJson::attributes2json(const string& element,
                                   string* value)
{
  string attributes;
  // If there is a space anywhere the next substring
  // has to be an attribute.
  size_t attNameStart = element.find(" ", 0);
  // If no space is found, the whole element is the value,
  // else everything before the space.
  if (attNameStart != string::npos) *value = element.substr(0, attNameStart);
  else *value = element; // NOLINT

  size_t attNameEnd;
  size_t attStart;
  size_t attEnd;
  while (attNameStart < element.size())
  {
    // Find the name of the attribute.
    while (attNameStart < element.size() && !isalpha(element[attNameStart]))
      attNameStart++;
    // If we come to the end to the string, we parsed all attributes.
    if (attNameStart == element.size()) break;
    attNameEnd = attNameStart + 1;
    while (isalpha(element[attNameEnd])) attNameEnd++;
    const string& name = element.substr(attNameStart,
                                        attNameEnd - attNameStart);

    // Find the quotes, which indicates the start and end of attribute textes.
    attStart = element.find("\"", attNameEnd);
    attEnd = element.find("\"", attStart + 1);
    const string& text = element.substr(attStart, attEnd - attStart + 1);
    if (!attributes.empty()) attributes.append(",");
    attributes.append("\"@" + name + "\":" + text);
    attNameStart = attEnd + 1;
  }
  return attributes;
}

// ____________________________________________________________________________
string XmlToJson::convertNodeToJson(const Node &tree)
{
  string json;
  // Attributes to json.
  json.append(tree.attributes);
  if (!json.empty() && !tree.children.empty()) json.append(",");

  // Children to json (recursivly).
  bool isMultipleChild = false;
  bool isListedAsMultiple = false;
  bool alreadyOccured = false;
  for (size_t i = 0; i < tree.children.size(); i++)
  {
    const Node &child = tree.children[i];

    // Check if this child already occured in the last iteration.
    alreadyOccured = isMultipleChild;

    // Check if this child occurs again in the next iteration.
    isMultipleChild = (i < tree.children.size() - 1
                       ? child.name == tree.children[i + 1].name
                       : false);

    // Check if this child shall always be handled as list.
    isListedAsMultiple = _multiples.count(child.name);

    // Child occurs the first time.
    if (!alreadyOccured) json.append("\"" + child.name + "\":");

    // It is a multiple, which occurs the first time, so open an array.
    if (!alreadyOccured && (isMultipleChild || isListedAsMultiple))
      json.append("[");

    // Append content of the tag (recursively).
    json.append(child.text);

    // If it did not already occur and it is no multiple child, but a listed
    // multiple. It must be an array with just one element. It is already
    // opened, now close it.
    if (isListedAsMultiple && !alreadyOccured && !isMultipleChild)
      json.append("]");

    // Child already occured, but won't occur again. Close it.
    if (alreadyOccured && !isMultipleChild) json.append("]");

    // As long this is not the last element, add a comma.
    // Commas for multiple childs are handled extra.
    if (i < tree.children.size() - 1) json.append(",");
  }

  // Text to json.
  bool justText = json.empty();
  if (!tree.text.empty())
  {
    string jsonText = "\"" + escapeInvalidChars(tree.text) + "\"";
    // Children and attributes have been empty.
    if (json.empty())
    {
      justText = true;
      json.append(jsonText);
    // Text is not the only object in this array.
    }
    else
    {
      json.append(",\"text\":" + jsonText);
    }
  }
  // In case of an empty text, add nothing or empty quotes, depending if it is
  // a multiple or not.
  bool isMultipleNode = _multiples.count(tree.name);
  if (json.empty() && !isMultipleNode) json.append("\"\"");
  if (!justText) json = "{" + json + "}";
  return json;
}

// ____________________________________________________________________________
XmlToJson::Node XmlToJson::getXmlTreeIterative(const string& xml)
{
  size_t position = 0;
  std::stack<Node*> xmlStack;
  Node root;
  root.name = "";
  xmlStack.push(&root);

  while (position < xml.length())
  {
    string name = "";
    string attributes = "";

    size_t startValue = position;
    while (startValue < xml.length() - 1 &&
        (xml[startValue] == ' '
        || xml[startValue] == '\n'
        || xml[startValue] == '\r')) startValue++;

    // If it doesn't exist and we are at the beginning of a text.
    if (xml[startValue] != '<')
    {
      startValue = xml.find("</" + xmlStack.top()->name, startValue + 1);
      xmlStack.top()->text = xml.substr(position, startValue - position);
      if (startValue == string::npos) break;
    }
    // If we find </ this had to be the end of the last tag.
    if (xml[startValue + 1] == '/')
    {
      position = startValue + xmlStack.top()->name.size() + 3;
      // children to json
      xmlStack.top()->text = convertNodeToJson(*xmlStack.top());
      xmlStack.pop();
      continue;
    } else if (isalpha(xml[startValue + 1]))
    {
      // Find next occurence of ">". But take care, since > can also occur
      // within attributes.
      bool isQuoted = false;
      size_t i = startValue + 1;
      while (i < xml.size())
      {
        if (xml[i] == '>' && !isQuoted) break;
        else if (xml[i] == '\"') isQuoted = !isQuoted;
        i++;
      }
      size_t endValue = i;

      // The substring between < and > is the element (with attributes)
      string element = xml.substr(startValue + 1,
                                  endValue - startValue - 1);

      // If the element contains a space it is actually has to be an attribute
      // with properties, so search for the end.
      Node node;
      node.attributes = attributes2json(element, &name);
      node.name = name;
      xmlStack.top()->children.push_back(node);
      xmlStack.push(&xmlStack.top()->children.back());
      position = endValue + 1;
    }
  }
  assert(!xmlStack.empty());
  return *xmlStack.top();
}

// ____________________________________________________________________________
string XmlToJson::escapeInvalidChars(const string &textToEscape)
{
  // Check for characters, which should be esacped.
  string text;
  text.reserve(textToEscape.size());
  for (size_t i = 0; i < textToEscape.length(); i++)
  {
    if ((textToEscape[i] == '\\' || textToEscape[i] == '\"'))
      text.append("\\");
    text.append(1, textToEscape[i]);
  }
  return text;
}


// ____________________________________________________________________________
void XmlToJson::printTree(const Node &node, int indent) const
{
  string indentStr = string(2 * indent, ' ');
  string attributeIndentStr = indentStr + "--";
  cout << indentStr << "Name: " << node.name << endl;
  cout << indentStr << "Text: " << node.text << endl;
  if (!node.attributes.empty())
    cout << indentStr << "Attributes:" << node.attributes << endl;
  if (!node.children.empty())
    cout << indentStr << "Children:" << endl;
  for (size_t i = 0; i < node.children.size(); i++)
    printTree(node.children[i], indent + 1);
}


