/* register_types.cpp */

#include "register_types.h"
#include "core/class_db.h"
#include "bullet_manager.h"

void register_bullet_manager_types() {
    ClassDB::register_class<Bullet>();
    ClassDB::register_class<BulletManager>();
	
}

void unregister_bullet_manager_types() {
   //nothing to do here
}