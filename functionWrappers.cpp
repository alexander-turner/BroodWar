int getDPS(Unit u) {
	return ((double) u->getType().groundWeapon().damageAmount() * u->getType().maxGroundHits()) / u->getType().groundWeapon().damageCooldown();
}

int getHP(Unit u) {
	return u->getHitPoints();
}

int getHPtoDPSratio(Unit u) {
	return getDPS(u) / getHP(u);
}