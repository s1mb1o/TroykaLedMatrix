#include "TroykaLedMatrix.h"

TroykaLedMatrix::TroykaLedMatrix() {
    _addr = I2C_ADDR_BASE | (I2C_ADDR_ALT0 & I2C_ADDR_MASK);
}

TroykaLedMatrix::TroykaLedMatrix(const uint8_t addr) {
    _addr = I2C_ADDR_BASE | (addr & I2C_ADDR_MASK);
}

void TroykaLedMatrix::_init() {
    _shutDown = false;
    _audioInput = false;
    _audioEqualizer = false;
    _audioInputGain = AUDIO_GAIN_0DB;
    //_currentLimit = ROW_CURRENT_30MA;
    setCurrentLimit(ROW_CURRENT_05MA);
    setMatrixSize(MATRIX_SIZE_8X8);
//    _wire->begin(); //We have to do this in setup
    _writeReg(REG_ADDR_CONFIGURATION, _makeConfigReg());
    _writeReg(REG_ADDR_LIGHTING_EFFECT, _makeEffectReg());
    disableEqualizer();
}

void TroykaLedMatrix::begin(TwoWire& wire) {
    _wire = &wire;
    _init();
}

void TroykaLedMatrix::begin() {
    begin(Wire);
}

void TroykaLedMatrix::enableDisplay() {
    _shutDown = false;
    uint8_t data = _makeConfigReg();
    _writeReg(REG_ADDR_CONFIGURATION, data);
}

void TroykaLedMatrix::disableDisplay() {
    _shutDown = true;
    uint8_t data = _makeConfigReg();
    _writeReg(REG_ADDR_CONFIGURATION, data);
}

void TroykaLedMatrix::enableAudioInput() {
    _audioInput = true;
    uint8_t data = _makeConfigReg();
    _writeReg(REG_ADDR_CONFIGURATION, data);
}

void TroykaLedMatrix::disableAudioInput() {
    _audioInput = false;
    uint8_t data = _makeConfigReg();
    _writeReg(REG_ADDR_CONFIGURATION, data);
}

void TroykaLedMatrix::enableEqualizer() {
    _audioEqualizer = true;
    _writeReg(REG_ADDR_AUDIO_EQUALIZER, _BV(BIT_AUDIO_EQ_EN));
}

void TroykaLedMatrix::disableEqualizer() {
    _audioEqualizer = false;
    _writeReg(REG_ADDR_AUDIO_EQUALIZER, 0);
}

void TroykaLedMatrix::setAudioGain(const uint8_t value) {
    _audioInputGain = value & AUDIO_GAIN_MASK;
    uint8_t data = _makeEffectReg();
    _writeReg(REG_ADDR_LIGHTING_EFFECT, data);
}

void TroykaLedMatrix::setCurrentLimit(const uint8_t value) {
    _currentLimit = value & ROW_CURRENT_MASK;
    uint8_t data = _makeEffectReg();
    _writeReg(REG_ADDR_LIGHTING_EFFECT, data);
}

void TroykaLedMatrix::setMatrixSize(const uint8_t value) {
    _matrixSize = (value & MATRIX_SIZE_MASK);
    uint8_t data = _makeConfigReg();
    _writeReg(REG_ADDR_CONFIGURATION, data);
    switch (_matrixSize) {
        case MATRIX_SIZE_8X8: { 
            _width = 8;
            _height = 8;
            break; 
        }
        case MATRIX_SIZE_7X9: {
            _width = 7;
            _height = 9;
            break;
        }
        case MATRIX_SIZE_6X10: {
            _width = 6;
            _height = 10;
            break; 
        }
        case MATRIX_SIZE_5X11: { 
            _width = 5;
            _height = 11;
            break;
        }
    }
}

void TroykaLedMatrix::clear() {
    for (uint8_t i = 0; i < MATRIX_MAX_ROWS; i++) {
        _data[i] = 0;
    }
    _updateDisplay();
}

void TroykaLedMatrix::clearPixel(const uint8_t x, const uint8_t y) {
    uint8_t i = x % 8;
    uint8_t j = y % MATRIX_MAX_ROWS;
    _data[j] = _data[j] & ~_BV(i);
}

void TroykaLedMatrix::drawPixel(const uint8_t x, const uint8_t y) {
    uint8_t i = x % 8;
    uint8_t j = y % MATRIX_MAX_ROWS;
    _data[j] = _data[j] | _BV(i);
    _updateDisplay();
}

void TroykaLedMatrix::selectFont(const uint8_t fontID) {
    switch (fontID) {
        default:
        case FONT_8X8_BASIC: {
            setFont((const uint8_t *)font8x8_basic, 128, 8);
            break;
        }
        case FONT_8X8_CONTROL: {
            setFont((const uint8_t *)font8x8_control, 32, 8);
            break;
        }
        case FONT_8X8_EXT_LATIN: {
            setFont((const uint8_t *)font8x8_ext_latin, 96, 8);
            break;
        }
        case FONT_8X8_GREEK: {
            setFont((const uint8_t *)font8x8_greek, 58, 8);
            break;
        }
        case FONT_8X8_MISC: {
            setFont((const uint8_t *)font8x8_misc, 10, 8);
            break;
        }
        case FONT_8X8_BOX: {
            setFont((const uint8_t *)font8x8_box, 128, 8);
            break;
        }
        case FONT_8X8_BLOCK: {
            setFont((const uint8_t *)font8x8_block, 32, 8);
            break;
        }
        case FONT_8X8_HIRAGANA: {
            setFont((const uint8_t *)font8x8_hiragana, 96, 8);
            break;
        }
        case FONT_8X8_SGA: {
            setFont((const uint8_t *)font8x8_sga, 26, 8);
            break;
        }
    }
}
void TroykaLedMatrix::setFont(const uint8_t* font, const uint8_t countChars, const uint8_t countRaws) {
//void TroykaLedMatrix::setFont(const uint8_t* PROGMEM font, const uint8_t countChars = 1, const uint8_t countRaws = 8) {
    _font = (uint8_t*)font;
    _fontSize = countChars;
    _fontHeight = countRaws;
}

void TroykaLedMatrix::drawSymbol(const uint8_t c) {
    if (_font) {
        if (c < _fontSize) {
            drawBitmapF(&_font[c * _fontHeight], _fontHeight);
        }
    }
    _updateDisplay();
}

//void TroykaLedMatrix::drawBitmap(const uint8_t* data, bool const reverse, const uint8_t countRaws = 8) {
void TroykaLedMatrix::drawBitmap(const uint8_t* data, bool const reverse, const uint8_t countRaws) {
    uint8_t n = min((uint8_t)countRaws, (uint8_t)MATRIX_MAX_ROWS);
    for (uint8_t i = 0; i < n; i++) {
        if (reverse) {
            _data[i] = pgm_read_byte(&RER_BIT_MAP[data[i]]);
        } else {
            _data[i] = data[i];
        }
    }
    _updateDisplay();
}

void TroykaLedMatrix::marquee(const uint8_t data[][8], const int len, const int sh, bool const reverse) {
    byte frame = sh / 8;
    byte col = sh % 8;
    for (uint8_t i = 0; i < 8; i++) {
        if (reverse) {
			byte line = data[frame % len][i] << col;
			line |= data[(frame+1) % len][i] >> (8 - col);
            _data[i] = pgm_read_byte(&RER_BIT_MAP[line]);
        } else {
			_data[i] = data[frame % len][i] << col;
            _data[i] |= data[(frame+1) % len][i] >> (8 - col);
        }
    }
    _updateDisplay();
}

//void TroykaLedMatrix::drawBitmapF(const uint8_t* PROGMEM data, const bool reverse, const uint8_t countRaws = 8) {
void TroykaLedMatrix::drawBitmapF(const uint8_t* data, const bool reverse, const uint8_t countRaws) {

    uint8_t n = min((uint8_t)countRaws, (uint8_t)MATRIX_MAX_ROWS);
    for (uint8_t i = 0; i < n; i++) {
        _data[i] = pgm_read_byte(&data[i]);
        //_data[i] = pgm_read_byte(data + i);
    }
    _updateDisplay();
}

byte TroykaLedMatrix::map(long input, long in_min, long in_max) {
	byte output = (input - in_min) * 8 / (in_max - in_min);
	return 0b11111111 >> output;
}

void TroykaLedMatrix::_updateDisplay() {
    //uint8_t w = _width;
    uint8_t h = _height;
    for (uint8_t i = 0; i < h; i++) {
        uint8_t data = _getRow(i);
        _writeReg(REG_ADDR_COLUMN_DATA + i, data);
        //_writeReg(REG_ADDR_COLUMN_DATA + i, pgm_read_byte(RER_BIT_MAP + data));
        //_writeReg(REG_ADDR_COLUMN_DATA + i, pgm_read_byte(&RER_BIT_MAP[data]);
    }
    _writeReg(REG_ADDR_UPDATE_COLUMN, 0xff);
}

uint8_t TroykaLedMatrix::_getRow(const uint8_t y) {
    return _data[y % MATRIX_MAX_ROWS];
}

uint8_t TroykaLedMatrix::_readReg(const uint8_t addr) {
    _wire->beginTransmission(_addr);
    _wire->write(addr);
    _wire->endTransmission();
    _wire->requestFrom(_addr, 1);
    while (!_wire->available());
    uint8_t data = _wire->read();
    return data; 
}

void TroykaLedMatrix::_writeReg(const uint8_t addr, const uint8_t data) {
    _wire->beginTransmission(_addr);
    _wire->write(addr);
    _wire->write(data);
    _wire->endTransmission();
}

uint8_t TroykaLedMatrix::_makeConfigReg() {
    uint8_t data = 0;
    if (_shutDown) {
        data |= _BV(BIT_CONFIG_SSD);
    }
    if (_audioInput) {
        data |= _BV(BIT_CONFIG_AUDIO_EN);
    }
    data |= _matrixSize << BIT_CONFIG_ADM;
    return data;
}

uint8_t TroykaLedMatrix::_makeEffectReg() {
    uint8_t data = (_audioInputGain << BIT_EFFECT_AUDIO_GAIN) |
                   (_currentLimit << BIT_EFFECT_ROW_CURRENT);
    return data;
}