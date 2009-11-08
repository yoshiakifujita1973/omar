var line_can = null;
var line_context = null;
var matcher_dom = null;
var origin_circle = null;
var current_circle = null;
var form_fields = null;

$(document).ready( function() {
	matcher_dom = id('matcher');
	line_can = id('lines');
	form_fields = id('form_fields');
	//line_can.height = matcher_dom.offsetHeight;
	//line_can.width = matcher_dom.offsetWidth;
	$('.circle').bind('mousedown',start_line).bind('mouseenter',set_circle).bind('mouseout',clear_circle);
	$(matcher_dom).bind('mousemove', draw_line).bind('mouseup',end_line);
	line_context = line_can.getContext('2d');
	$.get( 'data/circles.dat', create_circles );
});

function create_circles( data ) {
	var data_a = data.split('\n');
	for( var i = 0, c = data_a.length; i < c; i++ ) {
		var line_a = data_a[i].split(' ');
		var div = create('div');
		$(div).css({ 'height': line_a[2]*2+'px',
					 'width': line_a[2]*2+'px',
					 'left': (line_a[0]-line_a[2])+'px',
					 'top': (line_a[1]-line_a[2])+'px'
					 })
			.addClass('circle')
		var invisible_div = div.cloneNode(true);
		$(invisible_div).addClass('invisible').bind('mousedown',start_line).bind('mouseenter',set_circle).bind('mouseout',clear_circle);;
		form_fields.appendChild(div);
		form_fields.appendChild(invisible_div);
	}
}

function start_line( e ) {
	origin_circle = current_circle;
	var origin_circle_r = origin_circle.offsetWidth / 2;
	$(origin_circle).addClass( 'origin' );
		
	line_can.start_x = origin_circle.offsetLeft + origin_circle_r;
	line_can.start_y = offsetTop( origin_circle ) + origin_circle_r;
	line_context.save();
	return false;
}
function end_line( e ) {
	line_can.start_x = 0;
	line_can.start_y = 0;
	if( origin_circle && current_circle && origin_circle != current_circle ) {
		$([origin_circle,current_circle]).addClass( 'selected' );
	}
	$(origin_circle).removeClass( 'origin' );
	origin_circle = null;
	return false;
}
function draw_line( e ) {
	if( line_can.start_x ) {
		line_context.clearRect( 0, 0, 1000, 1000 );
		line_context.beginPath();
		line_context.moveTo( line_can.start_x, line_can.start_y );
		if( !current_circle ) {
			line_context.lineTo( e.pageX, e.pageY );
		} else {
			r = current_circle.offsetWidth / 2;
			line_context.lineTo( current_circle.offsetLeft + r, offsetTop( current_circle ) + r );
		}
		line_context.stroke();
	}
}
function set_circle( e ) {
	current_circle = e.target;
}
function clear_circle( e ) {
	current_circle = null;
}
function offsetTop( e ) {
	if( e == null ) {
		return 0;
	}
	return e.offsetTop + offsetTop( e.offsetParent );
}

function id(s) {return document.getElementById(s)}
function create(s) {return document.createElement(s)}