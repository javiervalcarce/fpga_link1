import 'dart:html';
import 'dart:convert';
import 'dart:async';

//         return HttpRequest.getString(_host + '/api1/sequence/$sequence_id/status');

rdClick(event) {
  Element target = event.target;
  var seq = target.getAttribute('seq');

  print('READ ${seq}');
}

wrClick(event) {
  Element target = event.target;
  var seq = target.getAttribute('seq');

  print('WRITE ${seq}');
}


void main() {

  NodeValidatorBuilder val = new NodeValidatorBuilder.common()
    ..allowNavigation()
    ..allowElement('TH', attributes: ['style'])
    ..allowElement('A', attributes: ['href', 'seq', 'download'])
    ..allowElement('BUTTON', attributes: ['seq'])
    ..allowElement('SPAN', attributes: ['style']);



  print('MemIO application');


  DivElement d = querySelector('#regs');

  String html;
  html  = '<table>';
  html += '<theader>';
  html += '<tr><td>#</td><td>Address</td><td>Value</td><td>Actions</td></tr>';
  html += '</theader>';
  html += '<tbody>';

  num i;
  
  for (i = 0; i < 10; i++) {
    html += '<tr> <td>${i}</td><td><input id="addr_${i}" type="number"></td> <td><input id="data_${i}"type="number"></td> <td><button id="rd_${i}" seq="${i}">Read</button> &nbsp; <button id="wr_${i}" seq="${i}">Write</buttom> </td> </tr>';
  }
  
  html += '</tbody>';
  html += '</table>';


  //d.innerHtml = html;
  d.setInnerHtml(html, validator: val);

  // Register event callbacks
  for (i = 0; i < 10; i++) {
    querySelector('#rd_${i}').onClick.listen(rdClick);
    querySelector('#wr_${i}').onClick.listen(wrClick);
  }
  
}
