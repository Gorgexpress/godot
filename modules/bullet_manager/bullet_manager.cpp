#include "bullet_manager.h"
#include "core/os/os.h"
#include "scene/2d/area_2d.h"
#include "scene/2d/collision_shape_2d.h"
#include "scene/scene_string_names.h"

void BulletManager::_notification(int p_what) {

	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {

		} break;

		case NOTIFICATION_READY: {
			set_physics_process(true);
			set_as_toplevel(true);
			_register_bullet_types();
		} break;
		case NOTIFICATION_DRAW: {
			if (texture.is_null())
				return;


			_draw_bullets();
		} break;

		case NOTIFICATION_PROCESS: {
		} break;
		case NOTIFICATION_PHYSICS_PROCESS: {
			_update_bullets();
		} break; 
	}
}

void BulletManager::set_shape(const Ref<Shape2D> &p_shape) {

	if (shape.is_valid())
		shape->disconnect("changed", this, "_shape_changed");
	shape = p_shape;
	update();

	if (shape.is_valid())
		shape->connect("changed", this, "_shape_changed");

	update_configuration_warning();
}

Ref<Shape2D> BulletManager::get_shape() const {

	return shape;
}

void BulletManager::set_collision_mask(uint32_t p_mask) {

	collision_mask = p_mask;
	//Physics2DServer::get_singleton()->area_set_collision_mask(get_rid(), p_mask);
}

uint32_t BulletManager::get_collision_mask() const {

	return collision_mask;
}

void BulletManager::set_collision_layer(uint32_t p_layer) {

	collision_layer = p_layer;
	//Physics2DServer::get_singleton()->area_set_collision_layer(get_rid(), p_layer);
}

uint32_t BulletManager::get_collision_layer() const {

	return collision_layer;
}
void BulletManager::set_texture(const Ref<Texture> &p_texture) {

	if (p_texture == texture)
		return;

	if (texture.is_valid())
		texture->remove_change_receptor(this);

	texture = p_texture;

	if (texture.is_valid())
		texture->add_change_receptor(this);

	update();
	emit_signal("texture_changed");
	item_rect_changed();
	_change_notify("texture");
}

void BulletManager::set_normal_map(const Ref<Texture> &p_texture) {

	normal_map = p_texture;
	update();
}

Ref<Texture> BulletManager::get_normal_map() const {

	return normal_map;
}

Ref<Texture> BulletManager::get_texture() const {

	return texture;
}

void BulletManager::set_vframes(int p_amount) {

	ERR_FAIL_COND(p_amount < 1);
	vframes = p_amount;
	update();
	item_rect_changed();
	_change_notify();
}
int BulletManager::get_vframes() const {

	return vframes;
}

void BulletManager::set_hframes(int p_amount) {

	ERR_FAIL_COND(p_amount < 1);
	hframes = p_amount;
	update();
	item_rect_changed();
	_change_notify();
}
int BulletManager::get_hframes() const {

	return hframes;
}

void BulletManager::set_frame(int p_frame) {

	ERR_FAIL_INDEX(p_frame, vframes * hframes);

	if (frame != p_frame)
		item_rect_changed();

	frame = p_frame;

	_change_notify("frame");
	emit_signal(SceneStringNames::get_singleton()->frame_changed);
}

int BulletManager::get_frame() const {

	return frame;
}

void BulletManager::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &BulletManager::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &BulletManager::get_texture);
	ClassDB::bind_method(D_METHOD("set_normal_map", "normal_map"), &BulletManager::set_normal_map);
	ClassDB::bind_method(D_METHOD("get_normal_map"), &BulletManager::get_normal_map);
	ClassDB::bind_method(D_METHOD("set_vframes", "vframes"), &BulletManager::set_vframes);
	ClassDB::bind_method(D_METHOD("get_vframes"), &BulletManager::get_vframes);
	ClassDB::bind_method(D_METHOD("set_hframes", "hframes"), &BulletManager::set_hframes);
	ClassDB::bind_method(D_METHOD("get_hframes"), &BulletManager::get_hframes);
	ClassDB::bind_method(D_METHOD("set_frame", "frame"), &BulletManager::set_frame);
	ClassDB::bind_method(D_METHOD("get_frame"), &BulletManager::get_frame);
	ClassDB::bind_method(D_METHOD("set_shape", "shape"), &BulletManager::set_shape);
	ClassDB::bind_method(D_METHOD("get_shape"), &BulletManager::get_shape);
	ClassDB::bind_method(D_METHOD("set_collision_layer", "collision_layer"), &BulletManager::set_collision_layer);
	ClassDB::bind_method(D_METHOD("get_collision_layer"), &BulletManager::get_collision_layer);
	ClassDB::bind_method(D_METHOD("set_collision_mask", "collision_mask"), &BulletManager::set_collision_mask);
	ClassDB::bind_method(D_METHOD("get_collision_mask"), &BulletManager::get_collision_mask);
	ClassDB::bind_method(D_METHOD("add_bullet", "position", "direction","speed", "acceleration", "rotation"), &BulletManager::add_bullet);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "normal_map", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_normal_map", "get_normal_map");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "vframes", PROPERTY_HINT_RANGE, "1,16384,1"), "set_vframes", "get_vframes");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "hframes", PROPERTY_HINT_RANGE, "1,16384,1"), "set_hframes", "get_hframes");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frame", PROPERTY_HINT_SPRITE_FRAME), "set_frame", "get_frame");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "shape", PROPERTY_HINT_RESOURCE_TYPE, "Shape2D"), "set_shape", "get_shape");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layer", PROPERTY_HINT_LAYERS_2D_PHYSICS), "set_collision_layer", "get_collision_layer");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_2D_PHYSICS), "set_collision_mask", "get_collision_mask");

}
void BulletManager::_get_rects(Rect2 &r_src_rect, Rect2 &r_dst_rect, bool &r_filter_clip) const {

	Rect2 base_rect;

	if (region) {
		r_filter_clip = region_filter_clip;
		base_rect = region_rect;
	} else {
		r_filter_clip = false;
		base_rect = Rect2(0, 0, texture->get_width(), texture->get_height());
	}

	Size2 frame_size = base_rect.size / Size2(hframes, vframes);
	Point2 frame_offset = Point2(frame % hframes, frame / hframes);
	frame_offset *= frame_size;
	r_src_rect.size = frame_size;
	r_src_rect.position = base_rect.position + frame_offset;

	Point2 dest_offset = offset;
	if (centered)
		dest_offset -= frame_size / 2;
	if (Engine::get_singleton()->get_use_pixel_snap()) {
		dest_offset = dest_offset.floor();
	}

	r_dst_rect = Rect2(dest_offset, frame_size);

	if (hflip)
		r_dst_rect.size.x = -r_dst_rect.size.x;
	if (vflip)
		r_dst_rect.size.y = -r_dst_rect.size.y;
}

void BulletManager::_update_bullets() {
	Physics2DServer *ps = Physics2DServer::get_singleton();
	float delta = get_physics_process_delta_time();
	int size = bullets.size();
	{
		PoolVector<Bullet*>::Read r = bullets.read();
		PoolVector<Bullet*>::Write w = bullets.write();
		int i = 0;
		while (i < size) {
			Bullet* bullet = r[i];
			if(bullet->has_collided) {
				ps->free(bullet->area);
				w[i] = r[size - 1];
				memdelete(bullet);
				size -= 1;
			}
			else {
				bullet->matrix[2] += bullet->direction * bullet->speed  * delta;
				bullet->speed += bullet->acceleration * delta;
				ps->area_set_transform(bullet->area, bullet->matrix);
			}
			i += 1;
		}
	}
	if (size != bullets.size()) {
		bullets.resize(size);
	}
	update();
}

void BulletManager::_draw_bullets() {
	if (texture.is_null() || bullets.size() == 0)
		return;
	RID ci = get_canvas_item();

	Rect2 src_rect, dst_rect;
	bool filter_clip;
	_get_rects(src_rect, dst_rect, filter_clip);
	Rect2 temp = dst_rect;
	PoolVector<Bullet*>::Read r = bullets.read();
	for(int i = 0; i < bullets.size(); i++) {
		Bullet* bullet = r[i];
		BulletType* type = bullet->type;
		if(!type->is_updated) {
			hframes = type->sprite->get_hframes();
			vframes = type->sprite->get_vframes();
			frame = type->sprite->get_frame();
			_get_rects(type->src_rect, type->dest_rect, filter_clip);
			type->is_updated = true;
		}
		dst_rect = type->dest_rect;
		dst_rect.position = bullet->matrix[2] + type->dest_rect.position;
		draw_texture_rect_region(type->sprite->get_texture(), dst_rect, type->src_rect, Color(1, 1, 1), false, normal_map, filter_clip);
	}
	for (Map<StringName, BulletType>::Element *E = types.front(); E; E = E->next()) {
		E->get().is_updated = false;
	}
	
}
void BulletManager::add_bullet(StringName type, Vector2 position, Vector2 direction,real_t speed, real_t acceleration) {
    Bullet* bullet(memnew(Bullet));


	bullet->direction = direction;
	bullet->speed = speed;
	bullet->acceleration = acceleration;
	bullet->matrix.elements[2] = position;
	bullet->type = &types[type];
	Physics2DServer *ps = Physics2DServer::get_singleton();
	RID area = ps->area_create();
	ps->area_attach_object_instance_id(area, bullet->get_instance_id());
	ps->area_set_collision_layer(area, 0);
	ps->area_set_collision_mask(area, 9);
	ps->area_set_monitor_callback(area, bullet, _body_inout_name);
	ps->area_set_area_monitor_callback(area, bullet, _area_inout_name);
	ps->area_set_transform(area, bullet->matrix);
	ps->area_add_shape(area, shape->get_rid());
	

	if (is_inside_tree()) {
		RID space = get_world_2d()->get_space();
		Physics2DServer::get_singleton()->area_set_space(area, space);
	}
	bullet->area = area;
	bullets.push_back(bullet);
}


void Bullet::_area_inout(int p_status, const RID &p_area, int p_instance, int p_area_shape, int p_self_shape) {
	has_collided = true;
	Physics2DServer::get_singleton()->area_set_collision_mask(area, 0);
	Physics2DServer::get_singleton()->area_set_collision_layer(area, 0);
	Object* collider = ObjectDB::get_instance(p_instance);
	collider->get_script_instance()->call("on_collision_bullet", 1);
}
void Bullet::_body_inout(int p_status, const RID &p_body, int p_instance, int p_body_shape, int p_area_shape) {
	has_collided = true;
	Physics2DServer::get_singleton()->area_set_collision_mask(area, 0);
	Physics2DServer::get_singleton()->area_set_collision_layer(area, 0);
}

void Bullet::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_body_inout"), &Bullet::_body_inout);
	ClassDB::bind_method(D_METHOD("_area_inout"), &Bullet::_area_inout);
}

void BulletManager::_register_bullet_types() {
	types = Map<StringName, BulletType>();
	for (int i = 0; i < get_child_count(); i++) {

		Node *child = get_child(i);

		if (!Object::cast_to<Sprite>(child)) {
			continue;
		}
		StringName name = child->get_name();
		if (types.has(name)) {
			print_error("Duplicate type " + name + " in BulletManager named " + get_name());
		}
		types[name] = BulletType();
		Sprite *s = Object::cast_to<Sprite>(child);
		types[name].sprite = s;
		CollisionShape2D *bullet_shape = Object::cast_to<CollisionShape2D>(s->get_child(0));
		types[name].shape = bullet_shape;
		types[name].name = name;
		types[name].is_updated = false;
	    //area->shape_owner_get_shape(0);
		//area->shape_owner_get_shape(area->get_shape_owners())
	}
}

