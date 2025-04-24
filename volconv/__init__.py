try:
    from ._volconv import convert, VolFormat
    __all__ = ["convert", "VolFormat"]
except ImportError:
    import warnings
    warnings.warn("Failed to import volconv. The extension module may not be properly built.")
    __all__ = []