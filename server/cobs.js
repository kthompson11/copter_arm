/**
 * attempts to unstuff a Buffer using consistent overhead byte stuffing (COBS)
 * data: COBS stuffed Buffer to unstuff; must have its terminating null byte
 * 
 * return: Buffer containing unstuffed data or false if input Buffer was invalid
 */
exports.cobs_unstuff = function (data) {
  if (data[0] == 0) {
    return false;
  }

  var unstuffed_data = new Uint8Array(data.length);

  var iSrc = 1;
  var iDest = 0;
  var count = data[0] - 1;

  while (iSrc < data.length - 1) {
    if (data[iSrc] === 0) {
      return false;
    }

    if (count === 0) {
      count = data[iSrc] - 1;
      iSrc += 1;
    } else {
      unstuffed_data[iDest] = data[iSrc];
      iDest += 1;
      iSrc += 1;
      count -= 1
    }
  }

  // if previous offset points to last byte then unstuffing is valid, otherwise invalid
  if (count === 0) {
    return Buffer.from(unstuffed_data.slice(0, iDest));
  } else {
    return false;
  }
}
