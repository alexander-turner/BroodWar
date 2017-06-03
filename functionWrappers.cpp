int getDPS(Unit u) {
	return ((double) u->getType().groundWeapon().damageAmount() * u->getType().maxGroundHits()) / u->getType().groundWeapon().damageCooldown();
}

int getHP(Unit u) {
	return u->getHitPoints();
}

int getDPStoHPRatio(Unit u) {
	return getDPS(u) / getHP(u);
}

// Returns the total HP difference between friendly and enemy units.
double getHPDiff(Unitset units, Unitset enemies) {
	double friendlyHP = 0;
	for (auto &u : units) {
		friendlyHP += getHP(u);
	}

	double enemyHP = 0;
	for (auto &u : enemies) {
		enemyHP += getHP(u);
	}

	return friendlyHP - enemyHP;
}