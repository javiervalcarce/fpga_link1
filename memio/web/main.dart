import 'dart:html';
import 'dart:convert';
import 'dart:async';

//         return HttpRequest.getString(_host + '/api1/sequence/$sequence_id/status');

class Register {
  int address;
  int value;

  Register(this.address, this.value);
}


List<Register> memory = new List<Register>();

NodeValidatorBuilder val = new NodeValidatorBuilder.common()
  ..allowNavigation()
  ..allowElement('THEADER', attributes: ['style'])
  ..allowElement('TBODY', attributes: ['style'])
  ..allowElement('TH', attributes: ['style'])
  ..allowElement('A', attributes: ['href', 'seq', 'download'])
  ..allowElement('BUTTON', attributes: ['seq'])
  ..allowElement('SPAN', attributes: ['style']);

void rdClick(Event e) {
  Element target = e.target;
  var seq = target.getAttribute('seq');

  print('READ ${seq}');

  int address = memory[seq].address;
  int data = memory[seq].value;

  SpanElement s = querySelector('#rdres_${seq}');

  s.setInnerHtml('?');
  s.style.color = 'black';


  HttpRequest.getString('/api1/reg/${address}').then((v) {
    print("request");

    s.innerHtml = 'ok';
    s.style.color = 'green';
    //querySelector('#data_${seq}').innerHtml = v;
  }).catchError((e) {
    print("rd error");

    s.innerHtml = 'error';
    s.style.color = 'red';
  });

}

void wrClick(Event e) {
  Element target = e.target;
  var seq = target.getAttribute('seq');

  print('WRITE ${seq}');

  int address = memory[seq].address;
  int data = memory[seq].value;

  SpanElement s = querySelector('#wrres_${seq}');

  s.setInnerHtml('?');
  s.style.color = 'black';

  HttpRequest.getString('/api1/reg/${address}/${data}').then((v) {
    print("request");
    s.innerHtml = 'ok';
    s.style.color = 'green';
  }).catchError((e) {
    print("wr error");
    s.innerHtml = 'error';
    s.style.color = 'red';
  });
}



void main() {

  memory.add(new Register(0, 100));
  memory.add(new Register(1, 200));
  memory.add(new Register(6, 200));
  memory.add(new Register(7, 200));
  memory.add(new Register(8, 200));
  memory.add(new Register(125, 5000));

  print('MemIO application');

  generateTable();

}

generateTable() {

  DivElement d = querySelector('#regs');

  String html;
  html  = '<table>';
  html += '<theader>';
  html += '<tr><td>#</td><td>Address</td><td>Value</td><td>Read</td><td>Write</td></tr>';
  html += '</theader>';
  html += '<tbody>';

  for (int i = 0; i < memory.length; i++) {

    int addr = memory[i].address;
    int data = memory[i].value;

    html += '<tr>';
    html += '<td>${i}</td>';
    html += '<td><input  id="addr_${i}" type="number" value="${addr}"></td>';
    html += '<td><input  id="data_${i}" type="number" value="${data}"></td>';

    html += '<td><button id="rdbtn_${i}" seq="${i}">Read </button> &nbsp; <span id="rdres_${i}">RRES</span> </td>';
    html += '<td><button id="wrbtn_${i}" seq="${i}">Write</button> &nbsp; <span id="wrres_${i}">WRES</span> </td>';
    html += '</tr>';
  }

  html += '</tbody>';
  html += '</table>';


  //d.innerHtml = html;
  d.setInnerHtml(html, validator: val);

  // Register event callbacks
  for (int i = 0; i < memory.length; i++) {
    querySelector('#rdbtn_${i}').onClick.listen(rdClick);
    querySelector('#wrbtn_${i}').onClick.listen(wrClick);
  }


}